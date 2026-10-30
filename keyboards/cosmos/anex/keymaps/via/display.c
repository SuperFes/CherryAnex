#include QMK_KEYBOARD_H
#include "logo.h"

#ifndef SPLASH_DURATION_MS
#    define SPLASH_DURATION_MS 2500
#endif

static void render_status(void) {
    switch (get_highest_layer(layer_state)) {
        case 0:
            oled_write_ln_P(PSTR("Main"), false);
            break;
        case 1:
            oled_write_ln_P(PSTR("Symbol"), false);
            break;
        case 2:
            oled_write_ln_P(PSTR("Games"), false);
            break;
        case 3:
            oled_write_ln_P(PSTR("RGB"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Insane!"), false);
    }

    led_t led_state = host_keyboard_led_state();
    oled_write_ln_P(led_state.num_lock ? PSTR("Num") : PSTR(""), false);
    oled_write_ln_P(led_state.caps_lock ? PSTR("Cap") : PSTR(""), false);
    oled_write_ln_P(led_state.scroll_lock ? PSTR("Scrl") : PSTR(""), false);
}

static void render_left(void) {
    render_status();
}

static void render_right(void) {
    render_status();
}

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
    if (is_keyboard_left()) {
        render_left();
    } else {
        render_right();
    }
    return false;
}
