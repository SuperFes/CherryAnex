# Anex firmware

QMK firmware for the Anex, a split RP2040 keyboard made with
[Cosmos](https://ryanis.cool/cosmos), with a 128x64 OLED on each half and a
VIA-editable keymap.

## Displays

Both halves show a status strip: the layer, active modifiers, lock keys and,
while [anex-host](host/README.md) runs on the PC, the time.

- **Left:** WPM with the session peak (or CPU and RAM use while anex-host
  runs) above a minute-long WPM graph.
- **Right:** lifetime and since-power-up keypress counts above the chip pet.

The pet walks or runs as you type and falls asleep when you stop. It also
reacts:

| Input | Reaction |
| --- | --- |
| Space | hops |
| Backspace | carefully steps back; five in a row makes it sweat |
| Ctrl held | crouches and sneaks |
| Caps Lock on | panics |
| A swear | a grawlix bubble, a soap scrub or a faint, picked at random |

After 30 seconds without typing the screensaver takes over both screens (a
starfield or the sleeping pet). After 5 minutes the screens and RGB switch
off; the next key brings everything back as it was.

## Special keys

These are VIA custom keycodes; load `via-definition.json` into VIA's Design
tab to see them by name.

| Key | Where | Does |
| --- | --- | --- |
| **Game** (`CUSTOM(0)`) | both thumb clusters, layer 0 | opens the game menu: snake or the chip runner |
| **Saver** (`CUSTOM(1)`) | layer 3, between T and `]` | switches screensaver style and previews it |

While a game is open no keys reach the PC.

- **Menu:** Up/Down or W/S to choose; Enter, Space or Right to start.
- **Snake:** arrows or WASD.
- **Runner:** Space, Up or W to jump; Down or S to drop.
- **Leaving:** Esc goes back to the menu, then out; Game leaves straight away.

## Building and flashing

Builds run in the same Docker image CI uses, against a QMK checkout:

```sh
QMK_HOME=/path/to/qmk_firmware tools/build.sh
```

That writes `anex_via_left.uf2` and `anex_via_right.uf2` here. CI pins QMK to
the tag in `.github/workflows/build.yml` (keep the local checkout on the same
tag) and attaches both files, plus anex-host for each OS, to the `latest`
release.

Each half needs its own file, since it records which side it is when flashed.
Put a half into its bootloader (double-tap its reset button, or `QK_BOOT` on
layer 3) and copy its file onto the `RPI-RP2` drive. In everyday use only the
left half should be on USB; with both plugged in each half acts alone and the
right one stops getting the left's state.

## Editing

| What | Edit | Then run |
| --- | --- | --- |
| Default keymap | save VIA's backup over `via-layout.json` | `tools/via2keymap.py` |
| Boot logo | `KB Logo.png` (128x64) | `tools/img2oled.py` |
| Pet, effects and runner art | `art/pet.txt`, `art/pet_fx.txt`, `art/runner.txt` | `tools/sprites.py` (with `--sheet` and `--prefix` for the last two) |
| Swear list | `art/badwords.txt`, not in git; see `art/badwords.example.txt` | `tools/badwords.py` |

Every new build resets VIA's stored keymap to the default, which is why the
default is generated from the VIA backup. Generated files say so at the top;
regenerate rather than edit them. The swear list never enters the repo: only
hashes of its words are committed.

## Tests

```sh
make -C tests          # firmware logic, built for the PC
python -m pytest tools # generators, and that generated files are current
cd host && cargo test  # anex-host
```

Firmware features keep their logic in plain C files with no QMK headers
(`snake.c`, `pet.c`, `saver.c`, ...) so they can be tested on the PC; files
like `game.c` and `display.c` connect them to QMK.

## Maybe

- **Layer cheat sheet:** while a layer key is held, each screen maps that
  layer's keys on its half.
- **Pet evolution:** the pet grows up as the lifetime keypress count passes
  milestones.
- **Audio visualiser:** anex-host sends spectrum bars of whatever is playing.
- **Two-player paddle game:** a paddle on each half, the ball crossing between
  the screens.
- **PC notifications:** unread counts, CI status or the next meeting, via
  anex-host.
- **Now playing:** track and artist from the PC; awkward to support on all
  three OSes.
- **Unplug warning:** tell you on screen when both halves are on USB.
- Stop the keypress total at its maximum instead of wrapping (about 4.3
  billion presses away).
- Fix the local `qmk` CLI (Python 3.14, missing colorama) for `qmk console`.
