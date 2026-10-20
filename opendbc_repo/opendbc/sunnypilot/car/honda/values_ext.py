"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

from enum import IntFlag

from opendbc.car.honda.values import CAR

# gas interceptor cars using the ISO 15622:2018 accel limit curve instead of NIDEC_ACCEL_MAX
HONDA_NIDEC_PEDAL_TUNE = {CAR.HONDA_CLARITY, CAR.HONDA_CIVIC}
PEDAL_ACCEL_MAX_BP = [0., 5., 20.]  # m/s
PEDAL_ACCEL_MAX_V = [4.0, 4.0, 3.0]  # m/s^2

# gas interceptor cars with a larger pedal deadzone, to prevent false gas pressed readings
HONDA_NIDEC_PEDAL_DEADZONE = {CAR.HONDA_ODYSSEY, CAR.HONDA_PILOT, CAR.HONDA_RIDGELINE}


class HondaFlagsSP(IntFlag):
  NIDEC_HYBRID = 1
  EPS_MODIFIED = 2
  HYBRID_ALT_BRAKEHOLD = 4


class HondaSafetyFlagsSP:
  NIDEC_HYBRID = 1
  GAS_INTERCEPTOR = 2
