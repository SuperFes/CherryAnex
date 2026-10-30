#pragma once

#include QMK_KEYBOARD_H

// HID usage a key sends when tapped, or KC_NO for non-basic keycodes.
static inline uint8_t key_usage(uint16_t keycode) {
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode) || IS_QK_MODS(keycode)) {
        return keycode & 0xFF;
    }
    return keycode <= 0xFF ? keycode : KC_NO;
}
