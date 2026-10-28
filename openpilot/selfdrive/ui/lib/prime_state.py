from enum import IntEnum
import os
import threading

from openpilot.common.params import Params
from openpilot.common.swaglog import cloudlog
from openpilot.selfdrive.selfdrived.alertmanager import set_offroad_alert


class PrimeType(IntEnum):
  UNKNOWN = -2
  UNPAIRED = -1
  NONE = 0
  MAGENTA = 1
  LITE = 2
  BLUE = 3
  MAGENTA_NEW = 4
  PURPLE = 5

class Provider(str):
  GOOGLE = "google"
  GITHUB = "github"
  APPLE = "apple"

class PrimeState:
  def __init__(self):
    self._params = Params()
    self._lock = threading.Lock()
    self.prime_type: PrimeType = self._load_initial_state()
    self._prime_trial_available = False
    pairing_provider = os.getenv("PAIRING_PROVIDER") or self._params.get("PairingProvider")
    self._pairing_provider: Provider | None = Provider(pairing_provider) if pairing_provider is not None else None
    self._pairing_email: str | None = self._params.get("PairingEmail")
    self._commacare = False

    # comma backend is disabled, never prompt to pair
    set_offroad_alert("Offroad_Pairing", False)

  def _load_initial_state(self) -> PrimeType:
    prime_type_str = os.getenv("PRIME_TYPE") or self._params.get("PrimeType")
    try:
      if prime_type_str is not None:
        return PrimeType(int(prime_type_str))
    except (ValueError, TypeError):
      pass
    return PrimeType.UNKNOWN

  def set_type(self, prime_type: PrimeType) -> None:
    with self._lock:
      if prime_type != self.prime_type:
        self.prime_type = prime_type
        self._params.put("PrimeType", int(prime_type))
        cloudlog.info(f"Prime type updated to {prime_type}")

  def set_provider(self, provider: Provider | None, email: str | None):
    with self._lock:
      if self.prime_type <= PrimeType.UNPAIRED:
        provider = None
        email = None
      email = email or None # if data.get(email) returns "" instead of None

      if self._pairing_provider != provider:
        self._pairing_provider = provider
        if provider is None:
          self._params.remove("PairingProvider")
        else:
          self._params.put("PairingProvider", str(provider))

      if self._pairing_email != email:
        self._pairing_email = email
        if email is None:
          self._params.remove("PairingEmail")
        else:
          self._params.put("PairingEmail", email)

  def set_commacare(self, has_commacare: bool):
    with self._lock:
      if self.prime_type <= PrimeType.UNPAIRED:
        self._commacare = False
      else:
        self._commacare = has_commacare

  def set_prime_trial_available(self, prime_trail_available: bool):
    with self._lock:
      if self.prime_type <= PrimeType.UNPAIRED:
        self._prime_trial_available = False
      else:
        self._prime_trial_available = prime_trail_available

  def get_type(self) -> PrimeType:
    with self._lock:
      return self.prime_type

  def is_prime(self) -> bool:
    with self._lock:
      return bool(self.prime_type > PrimeType.NONE)

  def is_full_prime(self) -> bool:
    with self._lock:
      return self.prime_type > PrimeType.NONE and self.prime_type != PrimeType.LITE

  def is_paired(self) -> bool:
    with self._lock:
      return self.prime_type > PrimeType.UNPAIRED

  def can_claim_prime_trial(self) -> bool:
    with self._lock:
      return self._prime_trial_available

  def has_commacare(self) -> bool:
    with self._lock:
      return self._commacare

  def get_pairing_provider(self) -> str | None:
    with self._lock:
      return self._pairing_provider

  def get_pairing_account(self) -> str:
    with self._lock:
      if not self._pairing_provider:
        return "unknown"
      elif self._pairing_provider == Provider.GITHUB or not self._pairing_email:
        return f"{self._pairing_provider} account"
      return self._pairing_email
