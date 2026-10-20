"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

import numpy as np

from opendbc.car import structs
from opendbc.car.can_definitions import CanData
from opendbc.sunnypilot.car import create_gas_interceptor_command

LongCtrlState = structs.CarControl.Actuators.LongControlState


class GasInterceptorCarController:
  def __init__(self, CP: structs.CarParams, CP_SP: structs.CarParamsSP):
    self.CP = CP
    self.CP_SP = CP_SP

    self.gas = 0.
    self.interceptor_gas_cmd = 0.

    # live-learned pedal gain and wind brake scale, kept in memory for the current drive
    self.gas_factor = 1.0
    self.wind_factor = 1.0
    self.wind_factor_before_brake = 1.0

  def update_pedal_learning(self, CC: structs.CarControl, CS: structs.CarState, gas: float, wind_brake: float) -> None:
    if CS.out.gasPressed or CC.actuators.longControlState != LongCtrlState.pid:
      return

    gas_error = CC.actuators.accel - CS.out.aEgo
    if gas_error != 0.0 and gas > 0.0:
      self.gas_factor = float(np.clip(self.gas_factor + gas_error / 150 * (gas * 4.8), 0.1, 3.0))
    if gas_error != 0.0 and not CS.out.brakePressed and CS.out.vEgo > 0.0:
      wind_adjust = 1 + (wind_brake * 4.8) / 1000
      self.wind_factor = float(np.clip(self.wind_factor * (wind_adjust if gas_error > 0 else 1.0 / wind_adjust), 0.1, 5.0))
    # don't reduce wind factor while braking, allow increases
    if gas <= 0.0:
      self.wind_factor = max(self.wind_factor, self.wind_factor_before_brake)
    else:
      self.wind_factor_before_brake = self.wind_factor

  def update(self, CC: structs.CarControl, CS: structs.CarState, gas: float, brake: float, wind_brake: float,
             packer, frame: int) -> list[CanData]:
    can_sends = []

    if self.CP_SP.enableGasInterceptor:
      self.update_pedal_learning(CC, CS, gas, wind_brake)

      # way too aggressive at low speed without this
      gas_mult = np.interp(CS.out.vEgo, [0., 10.], [0.4, 1.0])
      # send exactly zero if apply_gas is zero. Interceptor will send the max between read value and apply_gas.
      # This prevents unexpected pedal range rescaling
      # Sending non-zero gas when OP is not enabled will cause the PCM not to respond to throttle as expected
      # when you do enable.
      if CC.longActive:
        self.gas = float(np.clip(gas_mult * (gas * self.gas_factor - brake + wind_brake * 3 / 4), 0., 1.))
      else:
        self.gas = 0.0
      can_sends.append(create_gas_interceptor_command(packer, self.gas, frame // 2))

    return can_sends
