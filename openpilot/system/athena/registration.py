#!/usr/bin/env python3
import secrets
from pathlib import Path

from openpilot.common.params import Params
from openpilot.selfdrive.selfdrived.alertmanager import set_offroad_alert
from openpilot.common.hardware.hw import Paths


UNREGISTERED_DONGLE_ID = "UnregisteredDevice"

def is_registered_device() -> bool:
  dongle = Params().get("DongleId")
  return dongle not in (None, UNREGISTERED_DONGLE_ID)


def register(show_spinner=False) -> str:
  """
  Resolve the dongle ID locally without contacting any backend.
  Uses the existing param, then the factory ID in /persist/,
  and otherwise generates a random local ID.
  """
  params = Params()

  dongle_id: str | None = params.get("DongleId")
  if dongle_id in (None, "", UNREGISTERED_DONGLE_ID) and Path(Paths.persist_root()+"/comma/dongle_id").is_file():
    # not all devices will have this; added early in comma 3X production (2/28/24)
    with open(Paths.persist_root()+"/comma/dongle_id") as f:
      dongle_id = f.read().strip()

  if dongle_id in (None, "", UNREGISTERED_DONGLE_ID):
    dongle_id = secrets.token_hex(8)

  params.put("DongleId", dongle_id, block=True)
  set_offroad_alert("Offroad_UnregisteredHardware", False)
  return dongle_id


if __name__ == "__main__":
  print(register())
