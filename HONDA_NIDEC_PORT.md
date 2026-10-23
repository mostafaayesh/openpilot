# Honda Nidec port on release-tizi

This branch is commaai `release-tizi` plus:

- Nidec gas interceptor (pedal on `0x200`/`0x201`)
- `EPS_MODIFIED` detection and tunes (Civic, CR-V 5G)
- Nidec longitudinal tuning: grade compensation, `accel / 4.0` gas, brake modifier
- a custom panda firmware build, needed because comma's signed firmware blocks the pedal command

Learned pedal factors are saved to `/data/honda_nidec_learning.json` every 60 s while driving.
Delete that file to reset learning. They're kept out of Params on purpose: new Params keys need a
rebuilt `libparams_c.so`, and release branches ship that prebuilt.

## Rebasing onto a new release-tizi

comma's release branches are squashed, so the new release usually shares no history with the old
one. Don't `git rebase`. Move our commits across with `cherry-pick` instead.

```powershell
git fetch https://github.com/commaai/openpilot.git release-tizi
git log --oneline <old-base>..release-tizi-honda   # <old-base> = the "openpilot vX.Y.Z" commit under ours
git checkout -b release-tizi-honda-new FETCH_HEAD
git cherry-pick <old-base>..release-tizi-honda
```

**Panda firmware commit** (`panda: build H7 firmware ...`): don't merge these binaries. If it
conflicts, take the new release's files (`git checkout --theirs panda/board/obj; git add panda/board/obj;
git cherry-pick --continue`) and rebuild below. If you skip rebuilding, the stock firmware comes back:
brakes work, gas doesn't.

**After cherry-picking, check that comma didn't drop or move anything we rely on:**

- `CM_ "IMPORT _community.dbc";` is still the first line of the 7 Nidec DBCs in
  `opendbc_repo/opendbc/dbc/generator/honda/` (ilx_2016, rdx_2018, civic_touring_2016,
  clarity_hybrid_2018, crv_touring_2016, odyssey_exl_2018, odyssey_twn_2018)
- the modified EPS firmware strings `39990-TBA,A030` (Civic) and `39990-TLA,A040` (CR-V 5G)
  are in `opendbc_repo/opendbc/car/honda/fingerprints.py`
- `enableGasInterceptor` is still in `CarParams.deprecated` in `opendbc_repo/opendbc/car/car.capnp`
- `CHECKSUM_PEDAL` handling is still in `opendbc_repo/opendbc/can/dbc.py`

## Rebuilding the panda firmware (WSL Ubuntu)

One-time setup (`arm-none-eabi-gcc` is already installed in WSL):

```bash
python3 -m venv ~/.venvs/panda-build && ~/.venvs/panda-build/bin/pip install scons
```

Build and sign. `sign.py` has Windows line endings, so scons can't run it directly; call it with
`python3`:

```bash
cd /mnt/c/Users/mostafa/Dev/openpilot/panda
export PYTHONPATH=/mnt/c/Users/mostafa/Dev/openpilot/opendbc_repo
export GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=safe.directory GIT_CONFIG_VALUE_0='*'
~/.venvs/panda-build/bin/scons -j8 board/obj/bootstub.panda_h7.bin board/obj/panda_h7/main.bin
SETLEN=1 python3 board/crypto/sign.py board/obj/panda_h7/main.bin board/obj/panda_h7.bin.signed board/crypto/certs/debug
cat board/obj/version   # expect DEV-<commit>-DEBUG
```

Clean up and commit only the firmware. `panda/board/obj` is in `.gitignore`, but its files are
tracked, so `git add` still picks up changes:

```powershell
git checkout -- panda/board/obj/cert.h   # only line endings change
Remove-Item -Recurse -Force panda/.sconsign.dblite, panda/board/board, panda/board/crypto/board, panda/board/stm32h7/board
git add panda/board/obj
git commit -m "panda: build H7 firmware with Honda Nidec gas interceptor safety"
```

## Deploying

1. Run the Honda tests on the device:
   `cd /data/openpilot && PYTHONPATH=/data/openpilot /usr/local/venv/bin/python -m unittest opendbc.car.honda.tests.test_honda`
2. Replace the branch: `git push -f origin release-tizi-honda-new:release-tizi-honda`
3. On the device, while parked: `cd /data/openpilot && git fetch origin && git reset --hard origin/release-tizi-honda && sudo reboot`
4. pandad reflashes the panda on boot. Check that `/data/log/swaglog.*` contains `done flashing panda`.
   A failed flash can't brick the panda; it recovers through the ROM DFU loader.
