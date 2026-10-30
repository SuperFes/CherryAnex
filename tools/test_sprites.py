import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import sprites as s  # noqa: E402

RUNNER_SHEET = s.ROOT / "art/runner.txt"

SHEET = """
; comment
[one]
#.
..
..
..
..
..
..
.#

[two]
..
..
..
..
..
..
..
##
"""


class ParseTest(unittest.TestCase):
    def test_reads_sprites_in_order(self):
        sprites = s.parse(SHEET)
        self.assertEqual([name for name, _ in sprites], ["one", "two"])
        self.assertEqual(s.size(sprites), (2, 8))

    def test_packs_page_bytes(self):
        sprites = s.parse(SHEET)
        self.assertEqual(s.to_bytes(sprites[0][1]), bytes([0x01, 0x80]))
        self.assertEqual(s.to_bytes(sprites[1][1]), bytes([0x80, 0x80]))

    def test_rejects_stray_characters(self):
        with self.assertRaises(ValueError):
            s.parse("[a]\n#x\n")

    def test_rejects_pixels_before_name(self):
        with self.assertRaises(ValueError):
            s.parse("##\n")

    def test_rejects_mismatched_sizes(self):
        with self.assertRaises(ValueError):
            s.size(s.parse("[a]\n" + "#\n" * 8 + "[b]\n" + "##\n" * 8))

    def test_rejects_partial_page(self):
        with self.assertRaises(ValueError):
            s.size(s.parse("[a]\n" + "#\n" * 6))


class RenderTest(unittest.TestCase):
    def test_ids_enumerate_sprites(self):
        text = s.render_ids("pet", s.parse(SHEET), 2, 8, "x.txt")
        self.assertIn("PET_SPRITE_ONE,\n    PET_SPRITE_TWO,\n    PET_SPRITE_COUNT", text)
        self.assertIn("#define PET_SPRITE_WIDTH 2", text)

    def test_data_indexes_by_id(self):
        text = s.render_data("pet", s.parse(SHEET), 2, 8, "x.txt")
        self.assertIn("pet_sprites[PET_SPRITE_COUNT][2]", text)
        self.assertIn("[PET_SPRITE_TWO] = {\n        0x80, 0x80,", text)


class RepoFilesTest(unittest.TestCase):
    def test_pet_sprites_are_32_square(self):
        self.assertEqual(s.size(s.parse(s.DEFAULT_SHEET.read_text())), (32, 32))

    def test_runner_sprites_are_16_square(self):
        self.assertEqual(s.size(s.parse(RUNNER_SHEET.read_text())), (16, 16))

    def test_committed_headers_are_current(self):
        for sheet, prefix in ((s.DEFAULT_SHEET, s.DEFAULT_PREFIX), (RUNNER_SHEET, "runner")):
            for name, text in s.generate(sheet, prefix).items():
                self.assertEqual((s.KEYMAP / name).read_text(), text, f"{name}: run tools/sprites.py")


if __name__ == "__main__":
    unittest.main()
