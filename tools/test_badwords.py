import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import badwords as b  # noqa: E402


class HashTest(unittest.TestCase):
    # Same reference values as tests/test_words.c.
    def test_matches_firmware_hash(self):
        self.assertEqual(b.fnv1a(""), 0x811C9DC5)
        self.assertEqual(b.fnv1a("a"), 0xE40C292C)
        self.assertEqual(b.fnv1a("foobar"), 0xBF9CF968)


class ParseTest(unittest.TestCase):
    def test_splits_exact_and_prefix(self):
        exact, prefix = b.parse("# comment\nheck\nFrick*  # stem\n\n")
        self.assertEqual(exact, {"heck"})
        self.assertEqual(prefix, {"frick"})

    def test_rejects_non_letters(self):
        with self.assertRaises(ValueError):
            b.parse("he ck\n")
        with self.assertRaises(ValueError):
            b.parse("h3ck\n")

    def test_rejects_short_stems_and_long_words(self):
        with self.assertRaises(ValueError):
            b.parse("he*\n")
        with self.assertRaises(ValueError):
            b.parse("a" * (b.MAX_LEN + 1) + "\n")


class RenderTest(unittest.TestCase):
    def test_header_holds_only_sorted_hashes(self):
        text = b.render({"heck", "darn"}, {"frick"})
        self.assertNotIn("heck", text)
        self.assertNotIn("frick", text)
        hashes = sorted((b.fnv1a("heck"), b.fnv1a("darn")))
        self.assertIn(f"badword_exact[] = {{0x{hashes[0]:08x}, 0x{hashes[1]:08x}}};", text)
        self.assertIn("#define BADWORD_PREFIX_COUNT 1", text)

    def test_empty_lists_still_compile(self):
        text = b.render(set(), set())
        self.assertIn("badword_exact[] = {0};", text)
        self.assertIn("#define BADWORD_EXACT_COUNT 0", text)


class RepoFilesTest(unittest.TestCase):
    def test_example_list_parses(self):
        b.parse((b.ROOT / "art/badwords.example.txt").read_text())

    def test_committed_header_is_current(self):
        if not b.DEFAULT_LIST.exists():
            self.skipTest("art/badwords.txt is private and not in this checkout")
        self.assertEqual(b.main(["--check"]), 0, "run tools/badwords.py")


if __name__ == "__main__":
    unittest.main()
