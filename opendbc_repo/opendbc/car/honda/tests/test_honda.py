import os
import tempfile
import unittest

from opendbc.car import gen_empty_fingerprint
from opendbc.car.structs import CarParams
from opendbc.car.honda.carcontroller import NIDEC_BRAKE_MODIFIER_MAX, compute_gb_honda_nidec, load_nidec_learning, \
                                            save_nidec_learning, update_honda_nidec_brake_modifier
from opendbc.car.honda.interface import CarInterface
from opendbc.car.honda.values import CAR, CarControllerParams, HondaFlags, HondaSafetyFlags


def get_params(candidate, eps_fw=None, fingerprint=None):
  car_fw = [] if eps_fw is None else [CarParams.CarFw(ecu=CarParams.Ecu.eps, fwVersion=eps_fw, address=0x18da30f1)]
  return CarInterface.get_params(candidate, fingerprint or gen_empty_fingerprint(), car_fw, False, False, False)


class TestHondaFingerprint(unittest.TestCase):
  def test_tja_bosch_only(self):
    for car_model in CAR:
      if car_model.config.flags & HondaFlags.BOSCH_TJA_CONTROL:
        assert car_model.config.flags & HondaFlags.BOSCH, "Nidec car found with TJA control"


class TestHondaEpsModified(unittest.TestCase):
  def test_civic_stock_eps_uses_pid(self):
    CP = get_params(CAR.HONDA_CIVIC, b'39990-TBA-A030\x00\x00')
    assert CP.lateralTuning.which() == 'pid'
    assert not CP.flags & HondaFlags.EPS_MODIFIED
    assert not CP.dashcamOnly
    self.assertAlmostEqual(CP.lateralTuning.pid.kpV[0], 1.1, places=6)
    self.assertAlmostEqual(CP.lateralTuning.pid.kiV[0], 0.33, places=6)
    assert CarControllerParams(CP).STEER_MAX == 2560

  def test_civic_modified_eps_uses_pid(self):
    CP = get_params(CAR.HONDA_CIVIC, b'39990-TBA,A030\x00\x00')
    assert CP.lateralTuning.which() == 'pid'
    assert CP.flags & HondaFlags.EPS_MODIFIED
    assert not CP.dashcamOnly
    pid = CP.lateralTuning.pid
    self.assertAlmostEqual(pid.kpV[0], 0.1575, places=6)
    self.assertAlmostEqual(pid.kiV[0], 0.05175, places=6)
    self.assertAlmostEqual(pid.kf, 0.00004, places=9)

    params = CarControllerParams(CP)
    assert params.STEER_MAX == 15360
    assert params.STEER_LOOKUP_BP == [-15360, -2560, 0, 2560, 15360]
    assert params.STEER_LOOKUP_V == [-3840, -2560, 0, 2560, 3840]

  def test_crv_5g_modified_eps(self):
    CP = get_params(CAR.HONDA_CRV_5G, b'39990-TLA,A040\x00\x00')
    assert CP.lateralTuning.which() == 'pid'
    self.assertAlmostEqual(CP.lateralTuning.pid.kpV[0], 0.21, places=6)
    self.assertAlmostEqual(CP.lateralTuning.pid.kiV[0], 0.07, places=6)
    assert CarControllerParams(CP).STEER_LOOKUP_BP[-1] == 10000


class TestHondaNidecLongitudinal(unittest.TestCase):
  def test_gas_interceptor_detection(self):
    fingerprint = gen_empty_fingerprint()
    fingerprint[0][0x201] = 6
    CP = get_params(CAR.HONDA_CIVIC, fingerprint=fingerprint)
    assert CP.deprecated.enableGasInterceptor
    assert not CP.pcmCruise
    assert CP.safetyConfigs[-1].safetyParam & HondaSafetyFlags.GAS_INTERCEPTOR

    CP = get_params(CAR.HONDA_CIVIC)
    assert not CP.deprecated.enableGasInterceptor
    assert CP.pcmCruise

  def test_brake_modifier(self):
    modifier = 0.0
    for _ in range(200):
      modifier = update_honda_nidec_brake_modifier(modifier, -4.0)
    self.assertAlmostEqual(modifier, NIDEC_BRAKE_MODIFIER_MAX)
    # full brake at -4.0 m/s^2 once the modifier has ramped up
    _, brake = compute_gb_honda_nidec(-4.0, 10.0, modifier)
    self.assertAlmostEqual(brake, 1.0)
    assert update_honda_nidec_brake_modifier(modifier, -3.0) == 0.0

  def test_learning_persistence(self):
    with tempfile.TemporaryDirectory() as d:
      path = os.path.join(d, "learning.json")
      assert load_nidec_learning(path) == (1.0, 1.0)
      save_nidec_learning(path, 1.37, 0.82)
      gas, wind = load_nidec_learning(path)
      self.assertAlmostEqual(gas, 1.37)
      self.assertAlmostEqual(wind, 0.82)
      with open(path, "w") as f:
        f.write("{corrupt")
      assert load_nidec_learning(path) == (1.0, 1.0)
