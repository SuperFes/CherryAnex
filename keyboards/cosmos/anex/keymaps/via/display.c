#include QMK_KEYBOARD_H
#include "logo.h"
#include "status.h"

#ifndef SPLASH_DURATION_MS
#    define SPLASH_DURATION_MS 2500
#endif

static void write_status_line(const status_line_t *line) {
    // 21 columns fill the row, so the cursor wraps to the next line by itself.
    for (uint8_t col = 0; col < STATUS_COLS; ++col) {
        oled_write_char(line->text[col], line->invert & (1UL << col));
    }
}

// Two text rows plus a rule; side-specific content starts on row 3.
static void render_status_strip(void) {
    status_line_t line;
    uint8_t       mods = get_mods() | get_oneshot_mods();
    led_t         leds = host_keyboard_led_state();

    oled_set_cursor(0, 0);
    status_layer_line(&line, get_highest_layer(layer_state | default_layer_state),
                      (status_mods_t){
                          .ctrl  = mods & MOD_MASK_CTRL,
                          .shift = mods & MOD_MASK_SHIFT,
                          .alt   = mods & MOD_MASK_ALT,
                          .gui   = mods & MOD_MASK_GUI,
                      });
    write_status_line(&line);
    status_lock_line(&line, (status_locks_t){.caps = leds.caps_lock, .num = leds.num_lock, .scroll = leds.scroll_lock});
    write_status_line(&line);

    for (uint16_t x = 0; x < OLED_DISPLAY_WIDTH; ++x) {
        oled_write_raw_byte(0x08, 2 * OLED_DISPLAY_WIDTH + x);
    }
}

static void render_left(void) {}

static void render_right(void) {}

// Returns true while the boot logo still owns the screen.
static bool render_splash(void) {
    static bool done = false;

    if (done) {
        return false;
    }
    if (timer_read32() < SPLASH_DURATION_MS) {
        oled_write_raw_P(anex_logo, sizeof(anex_logo));
        return true;
    }
    done = true;
    oled_clear();
    return false;
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    if (render_splash()) {
        return false;
    }
    render_status_strip();
    if (is_keyboard_left()) {
        render_left();
    } else {
        render_right();
    }
    return false;
}
