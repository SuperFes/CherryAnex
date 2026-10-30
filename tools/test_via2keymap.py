import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import via2keymap as v  # noqa: E402

# 2x2 split matrix: left rows 0-1, right rows 2-3; (1,1) and (3,1) are unused positions.
TINY_KEYBOARD = {
    "matrix_pins": {"rows": ["GP0", "GP1"], "cols": ["GP2", "GP3"]},
    "split": {"enabled": True},
    "layouts": {"LAYOUT": {"layout": [{"matrix": m} for m in ([0, 0], [0, 1], [1, 0], [2, 0], [2, 1], [3, 0])]}},
}


def tiny_via(*layers):
    return {"layers": [list(layer) for layer in layers]}


def parse_layers(text):
    """Pull the LAYOUT() argument lists back out of generated C."""
    bodies = re.findall(r"LAYOUT\((.*?)\n    \)", text, re.S)
    return [[code.strip() for code in body.split(",") if code.strip()] for body in bodies]


class TranslateTest(unittest.TestCase):
    def test_legacy_names_map_to_current(self):
        self.assertEqual(v.translate("RESET"), "QK_BOOT")
        self.assertEqual(v.translate("RGB_TOG"), "UG_TOGG")
        self.assertEqual(v.translate("RGB_RMOD"), "UG_PREV")
        self.assertEqual(v.translate("KC_MS_BTN1"), "MS_BTN1")
        self.assertEqual(v.translate("KC_BTN3"), "MS_BTN3")

    def test_current_names_pass_through(self):
        for code in ("KC_A", "KC_TRNS", "MO(1)", "QK_BOOT", "UG_TOGG", "KC_CALC"):
            self.assertEqual(v.translate(code), code)

    def test_nested_codes_are_translated(self):
        self.assertEqual(v.translate("LT(1, KC_MS_BTN1)"), "LT(1, MS_BTN1)")

    def test_prefix_is_not_partially_replaced(self):
        self.assertEqual(v.translate("RGB_MODE_PLAIN"), "RGB_MODE_PLAIN")


class LayerRefsTest(unittest.TestCase):
    def test_layer_functions(self):
        self.assertEqual(v.layer_refs("MO(4)"), [4])
        self.assertEqual(v.layer_refs("LT(2, KC_SPC)"), [2])
        self.assertEqual(v.layer_refs("KC_A"), [])


class ViaToLayoutTest(unittest.TestCase):
    def setUp(self):
        self.rows, self.cols, self.positions = v.load_keyboard(TINY_KEYBOARD)

    def test_split_doubles_matrix_rows(self):
        self.assertEqual((self.rows, self.cols), (4, 2))

    def test_reorders_row_major_into_layout_order(self):
        via = tiny_via(["A", "B", "C", "KC_NO", "D", "E", "F", "KC_NO"])
        layers, warnings = v.via_to_layout(via, self.rows, self.cols, self.positions)
        self.assertEqual(layers, [["A", "B", "C", "D", "E", "F"]])
        self.assertEqual(warnings, [])

    def test_warns_on_code_at_unused_position(self):
        via = tiny_via(["A", "B", "C", "MO(1)", "D", "E", "F", "KC_TRNS"])
        _, warnings = v.via_to_layout(via, self.rows, self.cols, self.positions)
        self.assertEqual(len(warnings), 1)
        self.assertIn("(1,1)", warnings[0])

    def test_warns_on_missing_layer_target(self):
        via = tiny_via(["MO(1)", "B", "C", "KC_NO", "D", "E", "MO(2)", "KC_NO"], ["KC_TRNS"] * 8)
        _, warnings = v.via_to_layout(via, self.rows, self.cols, self.positions)
        self.assertEqual(warnings, ["layer 0: MO(2) targets layer 2, only 2 exist"])

    def test_rejects_wrong_layer_size(self):
        with self.assertRaises(ValueError):
            v.via_to_layout(tiny_via(["A"] * 7), self.rows, self.cols, self.positions)


class RenderTest(unittest.TestCase):
    def test_round_trips_and_has_no_trailing_comma(self):
        layers = [["KC_A", "KC_B", "KC_C"], ["KC_TRNS", "MO(1)", "UG_TOGG"]]
        text = v.render(layers, (2, 1), "x.json")
        self.assertEqual(parse_layers(text), layers)
        self.assertNotRegex(text, r",\s*\n    \)")

    def test_rejects_rows_not_matching_layout(self):
        with self.assertRaises(ValueError):
            v.render([["KC_A", "KC_B"]], (3,), "x.json")


class RepoFilesTest(unittest.TestCase):
    """Guards the real VIA backup against the real keyboard.json."""

    @classmethod
    def setUpClass(cls):
        cls.text, cls.warnings = v.generate(v.DEFAULT_VIA, v.DEFAULT_KEYBOARD)
        cls.layers = parse_layers(cls.text)

    def test_every_layer_has_all_88_keys(self):
        self.assertEqual(len(self.layers), 4)
        for layer in self.layers:
            self.assertEqual(len(layer), 88)

    def test_bottom_row_matches_via_positions(self):
        # LAYOUT index 69 is matrix (7,4), the key VIA shows as RAlt.
        self.assertEqual(self.layers[0][66:72], ["KC_LCTL", "KC_LGUI", "KC_LALT", "KC_RALT", "KC_APP", "KC_RCTL"])

    def test_no_legacy_names_survive(self):
        for code in (c for layer in self.layers for c in layer):
            for name in v.IDENT.findall(code):
                self.assertNotIn(name, v.LEGACY_ALIASES)

    def test_committed_keymap_is_current(self):
        self.assertEqual(v.DEFAULT_OUT.read_text(), self.text, "run tools/via2keymap.py")


if __name__ == "__main__":
    unittest.main()
