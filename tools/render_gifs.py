#!/usr/bin/env python3
"""Render the display gallery GIFs in docs/gifs from the firmware's own code.

Builds tools/oledsim against the keymap's QMK-free modules and QMK's OLED
font, runs every scene, and draws both OLEDs side by side, white on black.
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
KEYMAP = ROOT / "keyboards/cosmos/anex/keymaps/via"
SIM = ROOT / "tools/oledsim"
DEFAULT_OUT = ROOT / "docs/gifs"
MODULES = ["canvas", "graph", "pet", "runner", "saver", "snake", "status"]

WIDTH, HEIGHT = 128, 64
PANEL_BYTES = WIDTH * HEIGHT // 8
FRAME_BYTES = 2 + 2 * PANEL_BYTES
SCALE = 4  # unlit pixels drawn as smaller dots to hint at the panel grid
PAD, GAP = 12, 28

BEZEL, UNLIT, LIT = 0, 1, 2
PALETTE = [8, 8, 10, 22, 22, 26, 240, 244, 248]


def unpack(page_bytes):
    """Row-major booleans from an SSD1306 page-format buffer (LSB on top)."""
    pixels = [False] * (WIDTH * HEIGHT)
    for page in range(HEIGHT // 8):
        for x in range(WIDTH):
            byte = page_bytes[page * WIDTH + x]
            for bit in range(8):
                if byte >> bit & 1:
                    pixels[(page * 8 + bit) * WIDTH + x] = True
    return pixels


def read_frames(data):
    """(delay_ms, left, right) for each frame in a sim scene dump."""
    if len(data) % FRAME_BYTES:
        raise ValueError(f"{len(data)} bytes is not a whole number of frames")
    for at in range(0, len(data), FRAME_BYTES):
        (delay,) = struct.unpack_from("=H", data, at)
        left = data[at + 2 : at + 2 + PANEL_BYTES]
        right = data[at + 2 + PANEL_BYTES : at + FRAME_BYTES]
        yield delay, left, right


def collapse(frames):
    """Merges runs of identical frames into one held for their total delay."""
    out = []
    for delay, left, right in frames:
        if out and out[-1][1] == left and out[-1][2] == right:
            out[-1][0] += delay
        else:
            out.append([delay, left, right])
    return [tuple(frame) for frame in out]


def image_size():
    return 2 * WIDTH * SCALE + GAP + 2 * PAD, HEIGHT * SCALE + 2 * PAD


def compose(left, right):
    """Palette image of both panels; takes row-major booleans for each."""
    from PIL import Image

    width, height = image_size()
    out = bytearray(width * height)
    for origin, pixels in ((PAD, left), (PAD + WIDTH * SCALE + GAP, right)):
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # Lit pixels fill their cell so inverted text stays legible.
                lit = pixels[y * WIDTH + x]
                size = SCALE if lit else SCALE - 1
                for dy in range(size):
                    row = (PAD + y * SCALE + dy) * width + origin + x * SCALE
                    out[row : row + size] = bytes([LIT if lit else UNLIT]) * size
    image = Image.frombytes("P", (width, height), bytes(out))
    image.putpalette(PALETTE)
    return image


def write_gif(scene, out):
    frames = collapse(read_frames(scene.read_bytes()))
    images = [compose(unpack(left), unpack(right)) for _, left, right in frames]
    images[0].save(out, save_all=True, append_images=images[1:], duration=[f[0] for f in frames], loop=0, disposal=1)
    return len(images)


def build_sim(qmk_home, work):
    exe = work / "oledsim"
    cc = os.environ.get("CC", "cc")
    sources = [str(SIM / "sim.c")] + [str(KEYMAP / f"{m}.c") for m in MODULES]
    includes = [f"-I{SIM}", f"-I{KEYMAP}", f"-I{qmk_home / 'drivers/oled'}"]
    subprocess.run([cc, "-std=c11", "-O1", "-Wall", "-Wextra", "-Werror", *includes, "-o", str(exe), *sources], check=True)
    return exe


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qmk-home", type=Path, default=os.environ.get("QMK_HOME"), help="qmk_firmware checkout, for its OLED font (default: $QMK_HOME)")
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args(argv)
    if args.qmk_home is None:
        parser.error("set QMK_HOME or pass --qmk-home")

    args.out.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        exe = build_sim(args.qmk_home, work)
        scenes = work / "scenes"
        scenes.mkdir()
        subprocess.run([str(exe), str(scenes)], check=True)
        for scene in sorted(scenes.glob("*.bin")):
            out = args.out / f"{scene.stem}.gif"
            count = write_gif(scene, out)
            print(f"{out.relative_to(ROOT) if out.is_relative_to(ROOT) else out}: {count} frames")
    return 0


if __name__ == "__main__":
    sys.exit(main())
