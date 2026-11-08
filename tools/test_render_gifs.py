import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import render_gifs as r  # noqa: E402


def panel(**set_bytes):
    """A blank page-format panel with the given byte indexes (b<n>=value) set."""
    buf = bytearray(r.PANEL_BYTES)
    for key, value in set_bytes.items():
        buf[int(key[1:])] = value
    return bytes(buf)


def frame(delay, left, right):
    return struct.pack("=H", delay) + left + right


class UnpackTest(unittest.TestCase):
    def test_lsb_is_top_of_page(self):
        pixels = r.unpack(panel(b0=0x01))
        self.assertTrue(pixels[0])
        self.assertEqual(sum(pixels), 1)

    def test_msb_is_bottom_of_page(self):
        pixels = r.unpack(panel(b5=0x80))
        self.assertTrue(pixels[7 * r.WIDTH + 5])
        self.assertEqual(sum(pixels), 1)

    def test_pages_stack_downwards(self):
        pixels = r.unpack(panel(b130=0x01))
        self.assertTrue(pixels[8 * r.WIDTH + 2])


class ReadFramesTest(unittest.TestCase):
    def test_splits_delay_and_halves(self):
        left, right = panel(b0=1), panel(b1=2)
        frames = list(r.read_frames(frame(40, left, right) + frame(50, right, left)))
        self.assertEqual(frames, [(40, left, right), (50, right, left)])

    def test_rejects_partial_frame(self):
        with self.assertRaises(ValueError):
            list(r.read_frames(frame(40, panel(), panel())[:-1]))


class CollapseTest(unittest.TestCase):
    def test_holds_repeated_frames(self):
        a, b = panel(), panel(b0=1)
        frames = [(40, a, a), (40, a, a), (50, b, a), (40, a, a)]
        self.assertEqual(r.collapse(frames), [(80, a, a), (50, b, a), (40, a, a)])

    def test_either_half_changing_is_a_new_frame(self):
        a, b = panel(), panel(b0=1)
        self.assertEqual(len(r.collapse([(40, a, a), (40, a, b)])), 2)


class ComposeTest(unittest.TestCase):
    def test_places_both_panels(self):
        blank = [False] * (r.WIDTH * r.HEIGHT)
        lit = list(blank)
        lit[0] = True
        image = r.compose(lit, blank)
        self.assertEqual(image.size, r.image_size())
        self.assertEqual(image.getpixel((0, 0)), r.BEZEL)
        # Lit pixels fill their whole cell; unlit ones leave a gap.
        self.assertEqual(image.getpixel((r.PAD + r.SCALE - 1, r.PAD)), r.LIT)
        right_origin = r.PAD + r.WIDTH * r.SCALE + r.GAP
        self.assertEqual(image.getpixel((right_origin, r.PAD)), r.UNLIT)
        self.assertEqual(image.getpixel((right_origin + r.SCALE - 1, r.PAD)), r.BEZEL)


if __name__ == "__main__":
    unittest.main()
