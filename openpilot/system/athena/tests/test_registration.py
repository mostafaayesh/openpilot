from pathlib import Path

from openpilot.common.test import OpenpilotTestCase
from openpilot.common.params import Params
from openpilot.system.athena.registration import register, UNREGISTERED_DONGLE_ID
from openpilot.common.hardware.hw import Paths


class TestRegistration(OpenpilotTestCase):

  def setup_method(self):
    self.params = Params()

    persist_dir = Path(Paths.persist_root()) / "comma"
    persist_dir.mkdir(parents=True, exist_ok=True)

    self.dongle_id = persist_dir / "dongle_id"

  def test_valid_cache(self, mocker):
    # should work with a dongle ID on either /persist/ or normal params
    dongle = "DONGLE_ID_123"
    m = mocker.patch("requests.request", autospec=True)
    for persist, params in [(True, True), (True, False), (False, True)]:
      self.params.put("DongleId", dongle if params else "", block=True)
      with open(self.dongle_id, "w") as f:
        f.write(dongle if persist else "")
      assert register() == dongle
      assert not m.called

  def test_local_id(self, mocker):
    # no dongle id anywhere, generate a local one without touching the network
    m = mocker.patch("requests.request", autospec=True)
    dongle = register()
    assert not m.called
    assert dongle != UNREGISTERED_DONGLE_ID
    assert len(dongle) == 16
    assert self.params.get("DongleId") == dongle

    # stable across calls
    assert register() == dongle

  def test_replaces_unregistered(self):
    self.params.put("DongleId", UNREGISTERED_DONGLE_ID, block=True)
    dongle = register()
    assert dongle != UNREGISTERED_DONGLE_ID
    assert self.params.get("DongleId") == dongle
