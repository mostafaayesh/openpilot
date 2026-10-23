import math
import numpy as np

from opendbc.can import CANPacker
from opendbc.car import ACCELERATION_DUE_TO_GRAVITY, Bus, DT_CTRL, create_gas_interceptor_command, rate_limit, make_tester_present_msg, structs
from opendbc.car.honda import hondacan
from opendbc.car.honda.values import CAR, CruiseButtons, HondaFlags, CarControllerParams
from opendbc.car.interfaces import CarControllerBase
from opendbc.car.common.filter_simple import FirstOrderFilter

try:
  from openpilot.common.params import Params
except ImportError:
  Params = None

VisualAlert = structs.CarControl.HUDControl.VisualAlert
LongCtrlState = structs.CarControl.Actuators.LongControlState

NIDEC_BRAKE_MODIFIER_ACCEL = -3.9
NIDEC_BRAKE_MODIFIER_MAX = 0.8


def compute_gb_honda_bosch(accel, speed):
  # TODO returns 0s, is unused
  return 0.0, 0.0


def compute_gb_honda_nidec(accel, speed, brake_modifier=0.0):
  creep_brake = 0.0
  creep_speed = 2.3
  creep_brake_value = 0.15
  if speed < creep_speed:
    creep_brake = (creep_speed - speed) / creep_speed * creep_brake_value
  gas = float(accel) / 4.0 - creep_brake
  brake = float(accel) / (-4.8 + brake_modifier) + creep_brake
  return np.clip(gas, 0.0, 1.0), np.clip(brake, 0.0, 1.0)


def update_honda_nidec_brake_modifier(brake_modifier: float, accel: float) -> float:
  # ramp up brake strength while max deceleration is sustained, up to full brake at -4.0 m/s^2
  if accel < NIDEC_BRAKE_MODIFIER_ACCEL:
    return min(brake_modifier + 0.01, NIDEC_BRAKE_MODIFIER_MAX)
  return 0.0


def compute_gas_brake(accel, speed, CP, brake_modifier=0.0):
  if CP.flags & HondaFlags.BOSCH:
    return compute_gb_honda_bosch(accel, speed)
  else:
    return compute_gb_honda_nidec(accel, speed, brake_modifier)


# TODO not clear this does anything useful
def actuator_hysteresis(brake, braking, brake_steady):
  # hyst params
  brake_hyst_on = 0.02    # to activate brakes exceed this value
  brake_hyst_off = 0.005  # to deactivate brakes below this value
  brake_hyst_gap = 0.01   # don't change brake command for small oscillations within this value

  # *** hysteresis logic to avoid brake blinking. go above 0.1 to trigger
  if (brake < brake_hyst_on and not braking) or brake < brake_hyst_off:
    brake = 0.
  braking = brake > 0.

  # for small brake oscillations within brake_hyst_gap, don't change the brake command
  if brake == 0.:
    brake_steady = 0.
  elif brake > brake_steady + brake_hyst_gap:
    brake_steady = brake - brake_hyst_gap
  elif brake < brake_steady - brake_hyst_gap:
    brake_steady = brake + brake_hyst_gap
  brake = brake_steady

  return brake, braking, brake_steady


def brake_pump_hysteresis(apply_brake, apply_brake_last, last_pump_ts, ts):
  pump_on = False

  # reset pump timer if:
  # - there is an increment in brake request
  # - we are applying steady state brakes and we haven't been running the pump
  #   for more than 20s (to prevent pressure bleeding)
  if apply_brake > apply_brake_last or (ts - last_pump_ts > 20. and apply_brake > 0):
    last_pump_ts = ts

  # once the pump is on, run it for at least 0.2s
  if ts - last_pump_ts < 0.2 and apply_brake > 0:
    pump_on = True

  return pump_on, last_pump_ts


def process_hud_alert(hud_alert):
  alert_fcw = False
  alert_steer_required = False

  # Make sure FCW is prioritized over steering required
  # TODO: implement separate available LDW alert
  if hud_alert == VisualAlert.fcw:
    alert_fcw = True
  elif hud_alert in (VisualAlert.steerRequired, VisualAlert.ldw):
    alert_steer_required = True

  return alert_fcw, alert_steer_required


class CarController(CarControllerBase):
  def __init__(self, dbc_names, CP):
    super().__init__(dbc_names, CP)
    self.packer = CANPacker(dbc_names[Bus.pt])
    self.params = CarControllerParams(CP)
    self.param_store = Params() if Params is not None and CP.deprecated.enableGasInterceptor else None
    self.CAN = hondacan.CanBus(CP)
    self.tja_control = bool(CP.flags & HondaFlags.BOSCH_TJA_CONTROL)

    self.braking = False
    self.brake_steady = 0.
    self.brake_last = 0.
    self.apply_brake_last = 0
    self.last_pump_ts = 0.
    self.stopping_counter = 0

    self.accel = 0.0
    self.speed = 0.0
    self.gas = 0.0
    self.brake = 0.0
    self.last_torque = 0.0
    # smoothed pitch for Nidec grade compensation, so body pitch from braking/accelerating doesn't feed back
    self.nidec_pitch = FirstOrderFilter(0.0, 0.5, DT_CTRL)
    self.nidec_brake_modifier = 0.0

    # Nidec gas-pedal interceptor adaptation factors, persisted across drives
    self.bosch_gas_factor = 1.0
    self.bosch_wind_factor = 1.0
    if self.param_store is not None:
      try:
        self.bosch_gas_factor = float(self.param_store.get("HondaGasFactorParams") or 1.0)
        self.bosch_wind_factor = float(self.param_store.get("HondaWindFactorParams") or 1.0)
      except Exception:
        self.param_store = None
    self.bosch_wind_factor_before_brake = self.bosch_wind_factor
    self.bosch_gas_factor_before_gasmax = self.bosch_gas_factor
    self.bosch_wind_factor_before_gasmax = self.bosch_wind_factor

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators
    hud_control = CC.hudControl
    hud_v_cruise = hud_control.setSpeed / CS.v_cruise_factor if hud_control.speedVisible else 255
    pcm_cancel_cmd = CC.cruiseControl.cancel

    if len(CC.orientationNED) == 3:
      self.nidec_pitch.update(CC.orientationNED[1])
    nidec_hill_brake = math.sin(self.nidec_pitch.x) * ACCELERATION_DUE_TO_GRAVITY

    if CC.longActive:
      accel = actuators.accel
      # Nidec uses the grade-compensated request; Bosch is unaffected since compute_gb_honda_bosch is unused
      nidec_accel = actuators.accel + nidec_hill_brake
      self.nidec_brake_modifier = update_honda_nidec_brake_modifier(self.nidec_brake_modifier, nidec_accel)
      gas, brake = compute_gas_brake(nidec_accel, CS.out.vEgo, self.CP, self.nidec_brake_modifier)
    else:
      accel = 0.0
      gas, brake = 0.0, 0.0
      self.nidec_brake_modifier = 0.0

    # *** rate limit steer ***
    limited_torque = rate_limit(actuators.torque, self.last_torque, -self.params.STEER_DELTA_DOWN * DT_CTRL,
                                self.params.STEER_DELTA_UP * DT_CTRL)
    self.last_torque = limited_torque

    # *** apply brake hysteresis ***
    pre_limit_brake, self.braking, self.brake_steady = actuator_hysteresis(brake, self.braking, self.brake_steady)

    # *** rate limit after the enable check ***
    brake_rate_up = DT_CTRL if self.CP.flags & HondaFlags.BOSCH else 3 * DT_CTRL
    self.brake_last = rate_limit(pre_limit_brake, self.brake_last, -2., brake_rate_up)

    # vehicle hud display, wait for one update from 10Hz 0x304 msg
    alert_fcw, alert_steer_required = process_hud_alert(hud_control.visualAlert)

    # **** process the car messages ****

    # steer torque is converted back to CAN reference (positive when steering right)
    apply_torque = int(np.interp(-limited_torque * self.params.STEER_MAX, self.params.STEER_LOOKUP_BP, self.params.STEER_LOOKUP_V))

    # Send CAN commands
    can_sends = []

    # tester present - w/ no response (keeps radar disabled)
    if self.CP.flags & HondaFlags.BOSCH and not (self.CP.flags & HondaFlags.BOSCH_RADARLESS) and self.CP.openpilotLongitudinalControl:
      if self.frame % 10 == 0:
        can_sends.append(make_tester_present_msg(0x18DAB0F1, 1, suppress_response=True))

    # Send steering command.
    can_sends.append(hondacan.create_steering_control(self.packer, self.CAN, apply_torque, CC.latActive, self.tja_control))

    # wind brake from air resistance decel at high speed
    wind_brake = np.interp(CS.out.vEgo, [0.0, 2.3, 35.0], [0.001, 0.002, 0.15])
    # all of this is only relevant for HONDA NIDEC
    max_accel = np.interp(CS.out.vEgo, self.params.NIDEC_MAX_ACCEL_BP, self.params.NIDEC_MAX_ACCEL_V)
    # TODO this 1.44 is just to maintain previous behavior
    pcm_speed_BP = [-wind_brake,
                    -wind_brake * (3 / 4),
                    0.0,
                    0.5]
    # The Honda ODYSSEY seems to have different PCM_ACCEL
    # msgs, is it other cars too?
    gas_interceptor_command = 0.0
    if not CC.longActive:
      pcm_speed = 0.0
      pcm_accel = int(0.0)
    elif self.CP.deprecated.enableGasInterceptor:
      pcm_speed = 0.0
      pcm_accel = int(0.0)
    elif self.CP.flags & HondaFlags.NIDEC_ALT_PCM_ACCEL:
      pcm_speed_V = [0.0,
                     np.clip(CS.out.vEgo - 3.0, 0.0, 100.0),
                     np.clip(CS.out.vEgo + 0.0, 0.0, 100.0),
                     np.clip(CS.out.vEgo + 5.0, 0.0, 100.0)]
      pcm_speed = float(np.interp(gas - brake, pcm_speed_BP, pcm_speed_V))
      pcm_accel = int(1.0 * self.params.NIDEC_GAS_MAX)
    else:
      pcm_speed_V = [0.0,
                     np.clip(CS.out.vEgo - 2.0, 0.0, 100.0),
                     np.clip(CS.out.vEgo + 2.0, 0.0, 100.0),
                     np.clip(CS.out.vEgo + 5.0, 0.0, 100.0)]
      pcm_speed = float(np.interp(gas - brake, pcm_speed_BP, pcm_speed_V))
      pcm_accel = int(np.clip((accel / 1.44) / max_accel, 0.0, 1.0) * self.params.NIDEC_GAS_MAX)

    if not self.CP.openpilotLongitudinalControl:
      if self.frame % 2 == 0 and not (self.CP.flags & (HondaFlags.BOSCH_RADARLESS | HondaFlags.BOSCH_CANFD)):
        can_sends.append(hondacan.create_bosch_supplemental_1(self.packer, self.CAN))
      # If using stock ACC, spam cancel command to kill gas when OP disengages.
      if pcm_cancel_cmd:
        can_sends.append(hondacan.spam_buttons_command(self.packer, self.CAN, CruiseButtons.CANCEL, self.CP))
      elif CC.cruiseControl.resume:
        can_sends.append(hondacan.spam_buttons_command(self.packer, self.CAN, CruiseButtons.RES_ACCEL, self.CP))

    else:
      # Send gas and brake commands.
      if self.frame % 2 == 0:
        ts = self.frame * DT_CTRL

        if self.CP.flags & HondaFlags.BOSCH:
          self.accel = float(np.clip(accel, self.params.BOSCH_ACCEL_MIN, self.params.BOSCH_ACCEL_MAX))
          self.gas = float(np.interp(accel, self.params.BOSCH_GAS_LOOKUP_BP, self.params.BOSCH_GAS_LOOKUP_V))

          stopping = actuators.longControlState == LongCtrlState.stopping
          self.stopping_counter = self.stopping_counter + 1 if stopping else 0
          can_sends.extend(hondacan.create_acc_commands(self.packer, self.CAN, CC.enabled, CC.longActive, self.accel, self.gas,
                                                        self.stopping_counter, self.CP))
        else:
          apply_brake = np.clip(self.brake_last - wind_brake, 0.0, 1.0)
          apply_brake = int(np.clip(apply_brake * self.params.NIDEC_BRAKE_MAX, 0, self.params.NIDEC_BRAKE_MAX - 1))
          pump_on, self.last_pump_ts = brake_pump_hysteresis(apply_brake, self.apply_brake_last, self.last_pump_ts, ts)

          pcm_override = True
          can_sends.append(hondacan.create_brake_command(self.packer, self.CAN, apply_brake, pump_on,
                                                         pcm_override, pcm_cancel_cmd, alert_fcw, CS.stock_brake))
          self.apply_brake_last = apply_brake
          self.brake = apply_brake / self.params.NIDEC_BRAKE_MAX

          if self.CP.deprecated.enableGasInterceptor:
            gas_error = actuators.accel - CS.out.aEgo

            if not CS.out.gasPressed and actuators.longControlState == LongCtrlState.pid:
              if gas_error != 0.0 and gas > 0.0:
                self.bosch_gas_factor = float(np.clip(self.bosch_gas_factor + gas_error / 150.0 * (gas * 4.8), 0.1, 3.0))
              if gas_error != 0.0 and not CS.out.brakePressed and CS.out.vEgo > 0.0:
                wind_adjust = 1.0 + (wind_brake * 4.8) / 1000.0
                if gas_error > 0.0:
                  self.bosch_wind_factor = float(np.clip(self.bosch_wind_factor * wind_adjust, 0.1, 5.0))
                else:
                  self.bosch_wind_factor = float(np.clip(self.bosch_wind_factor / wind_adjust, 0.1, 5.0))
              if gas <= 0.0:
                self.bosch_wind_factor = max(self.bosch_wind_factor, self.bosch_wind_factor_before_brake)
              else:
                self.bosch_wind_factor_before_brake = self.bosch_wind_factor

            gas_mult = float(np.interp(CS.out.vEgo, [0.0, 10.0], [0.4, 1.0]))
            if CC.longActive:
              gas_interceptor_command = float(np.clip(
                gas_mult * ((gas * self.bosch_gas_factor) - brake + (wind_brake * self.bosch_wind_factor * 3.0 / 4.0)),
                0.0,
                1.0,
              ))
            idx = (self.frame // 2) % 0x10
            can_sends.append(create_gas_interceptor_command(self.packer, gas_interceptor_command, idx))

    # Send dashboard UI commands.
    if self.frame % 10 == 0:
      if self.CP.openpilotLongitudinalControl:
        # On Nidec, this also controls longitudinal positive acceleration
        can_sends.append(hondacan.create_acc_hud(self.packer, self.CAN.pt, self.CP, CC.enabled, pcm_speed, pcm_accel,
                                                 hud_control, hud_v_cruise, CS.is_metric, CS.acc_hud))

      steering_available = CS.out.cruiseState.available and CS.out.vEgo > max(self.params.STEER_GLOBAL_MIN_SPEED, self.CP.minSteerSpeed)
      can_sends.extend(hondacan.create_lkas_hud(self.packer, self.CAN.lkas, self.CP, hud_control, CC.latActive,
                                                steering_available, alert_steer_required, CS.lkas_hud))

      if self.CP.openpilotLongitudinalControl:
        # TODO: combining with create_acc_hud block above will change message order and will need replay logs regenerated
        if self.CP.flags & HondaFlags.BOSCH and not (self.CP.flags & HondaFlags.BOSCH_RADARLESS):
          can_sends.append(hondacan.create_radar_hud(self.packer, self.CAN.pt))
        if self.CP.carFingerprint == CAR.HONDA_CIVIC_BOSCH:
          can_sends.append(hondacan.create_legacy_brake_command(self.packer, self.CAN.pt))
        if not (self.CP.flags & HondaFlags.BOSCH):
          self.speed = pcm_speed
          if self.CP.deprecated.enableGasInterceptor:
            self.gas = gas_interceptor_command
          else:
            self.gas = pcm_accel / self.params.NIDEC_GAS_MAX

    if self.frame > 0 and self.frame % 6000 == 0 and self.param_store is not None:
      self.param_store.put("HondaGasFactorParams", float(round(self.bosch_gas_factor, 6)))
      self.param_store.put("HondaWindFactorParams", float(round(self.bosch_wind_factor, 6)))

    new_actuators = actuators.as_builder()
    new_actuators.speed = self.speed
    new_actuators.accel = self.accel
    new_actuators.gas = self.gas
    new_actuators.brake = self.brake
    new_actuators.torque = self.last_torque
    new_actuators.torqueOutputCan = apply_torque

    self.frame += 1
    return new_actuators, can_sends
