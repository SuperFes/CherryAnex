import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import img2oled as i  # noqa: E402


def blank(width, height):
    return [False] * (width * height)


class PackTest(unittest.TestCase):
    def test_top_left_pixel_is_lsb_of_first_byte(self):
        pixels = blank(2, 8)
        pixels[0] = True
        self.assertEqual(i.pack(pixels, 2, 8), bytes([0x01, 0x00]))

    def test_bottom_of_page_is_msb(self):
        pixels = blank(2, 8)
        pixels[7 * 2 + 1] = True
        self.assertEqual(i.pack(pixels, 2, 8), bytes([0x00, 0x80]))

    def test_second_page_follows_first(self):
        pixels = blank(1, 16)
        pixels[8] = True
        self.assertEqual(i.pack(pixels, 1, 16), bytes([0x00, 0x01]))

    def test_rejects_partial_page(self):
        with self.assertRaises(ValueError):
            i.pack(blank(4, 6), 4, 6)

    def test_rejects_wrong_pixel_count(self):
        with self.assertRaises(ValueError):
            i.pack(blank(4, 7), 4, 8)


class RenderTest(unittest.TestCase):
    def test_declares_dimensions_and_bytes(self):
        text = i.render("logo", bytes(range(20)), 4, 8, "x.png")
        self.assertIn("#define LOGO_WIDTH 4", text)
        self.assertIn("#define LOGO_HEIGHT 8", text)
        self.assertEqual(len(re.findall(r"0x[0-9a-f]{2},", text)), 20)


class RepoFilesTest(unittest.TestCase):
    def test_logo_fills_the_display(self):
        _, width, height = i.load(i.DEFAULT_IN)
        self.assertEqual((width, height), (128, 64))

    def test_committed_header_is_current(self):
        self.assertEqual(i.DEFAULT_OUT.read_text(), i.generate(i.DEFAULT_IN, i.DEFAULT_NAME), "run tools/img2oled.py")


if __name__ == "__main__":
    unittest.main()
