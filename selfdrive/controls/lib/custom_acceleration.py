import numpy as np

MAX_ISO_ACCEL_LOW_SPEED = 4.0
MAX_ISO_ACCEL_STEADY_STATE = 2.0
MINIMUM_LATERAL_ACCELERATION = 0.2

def get_max_allowed_accel(v_ego):
  # Simplified implementation using linear interpolation
  return np.interp(v_ego, [0., 20.], [MAX_ISO_ACCEL_LOW_SPEED, MAX_ISO_ACCEL_STEADY_STATE])
