#include "game.h"

static bool    active;
static bool    dirty; // board changed outside snake_update
static snake_t snake;

bool game_active(void) {
    return active;
}

const snake_t *game_snake(void) {
    return &snake;
}

static void start(void) {
    active = true;
    // Releases of keys held on entry would otherwise be swallowed, leaving them stuck on the host.
    clear_keyboard();
    snake_init(&snake, timer_read32());
    dirty = true;
}

static uint8_t basic_keycode(uint16_t keycode) {
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode) || IS_QK_MODS(keycode)) {
        return keycode & 0xFF;
    }
    return keycode <= 0xFF ? keycode : KC_NO;
}

bool game_record(uint16_t keycode, keyrecord_t *record) {
    if (!active) {
        if (keycode == GAME_TOGGLE && record->event.pressed) {
            start();
            return true;
        }
        return false;
    }
    if (!record->event.pressed) {
        return true;
    }

    uint8_t usage = basic_keycode(keycode);
    if (keycode == GAME_TOGGLE || usage == KC_ESC) {
        active = false;
        return true;
    }
    if (snake_can_restart(&snake, timer_read32())) {
        snake_init(&snake, timer_read32());
        dirty = true;
    }
    snake_dir_t dir;
    if (snake_dir_for_usage(usage, &dir)) {
        snake_turn(&snake, dir, timer_read32());
    }
    return true;
}

bool game_task(void) {
    if (!active) {
        return false;
    }
    bool changed = snake_update(&snake, timer_read32()) || dirty;
    dirty        = false;
    return changed;
}
