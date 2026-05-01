#include QMK_KEYBOARD_H

/* Running config
[1] = LAYOUT(
      KC_TRNS, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,                  KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,  KC_PSCR,
      KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F11,                  KC_F12,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_TRNS,
      KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_QUOT, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS,   KC_TRNS, KC_TRNS, KC_LBRC, KC_RBRC, KC_BSLS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS, KC_SPC,                KC_BSPC, KC_TRNS, KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3, KC_TRNS, KC_TRNS, KC_ENT, KC_TRNS,         KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3, KC_TRNS, KC_TRNS, KC_DEL, KC_TRNS
),
[2] = LAYOUT(
      KC_CALC, KC_MAIL, KC_HELP, KC_STOP, KC_ERAS, KC_AGAIN, KC_MENU,                KC_UNDO, KC_SELECT, KC_EXECUTE, KC_CUT, KC_COPY, KC_PASTE, KC_FIND,
      KC_TRNS, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,                  KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,  KC_TRNS,
      KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TRNS,
      KC_TRNS, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_TRNS,                 KC_TRNS, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SPC,                  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_ENT,  KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
),
[3] = LAYOUT(
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RGB_RMOD,                RGB_MOD, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      RGB_VAD, RGB_VAI, RGB_SPI, RGB_SPD, RESET,   RGB_MOD, KC_TRNS,                 RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RESET,   RGB_RMOD,KC_TRNS
)
*/
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,     /* <-> */    KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_BSPC,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_EQL,    /* <-> */    KC_LBRC, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    RGB_TOG,   /* <-> */    KC_RBRC, KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,               /* <-> */             KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,               /* <-> */             KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT,          MO(1),                       /* <-> */                      MO(1),            KC_RALT, KC_APP,  KC_RCTL,
        MO(2),   MO(3),   MO(4),   KC_HOME, KC_END,  KC_DEL,  KC_BSPC,   /* <-> */    MO(4),  MO(3),    MO(2),   KC_PGDN, KC_PGUP, KC_ENT,  KC_SPC
    ),
    [1] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          /* <-> */          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          /* <-> */          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            /* <-> */                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [2] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          /* <-> */          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          /* <-> */          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            /* <-> */                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, /* <-> */ KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
};

void keyboard_post_init_user(void) {
    debug_enable=true;
    debug_matrix=true;
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    // Host Keyboard Layer Status
    switch (get_highest_layer(layer_state)) {
    case 0:
        oled_write_P(PSTR("Main\n"), false);
        break;
    case 1:
        oled_write_P(PSTR("Symbol\n"), false);
        break;
    case 2:
        oled_write_P(PSTR("Games\n"), false);
        break;
    default:
        // Or use the write_ln shortcut over adding '\n' to the end of your string
        oled_write_ln_P(PSTR("Insane!"), false);
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("Num\n") : PSTR("\n"), false);
    oled_write_P(led_state.caps_lock ? PSTR("Cap\n") : PSTR("\n"), false);
    oled_write_P(led_state.scroll_lock ? PSTR("Scrl\n") : PSTR("\n"), false);

    return false;
}
/*
static void render_logo(void) {
    static const unsigned char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,240,248,248,252,252,254,254,254,254,254,254,254,254,254,254,254,254,254,254,252,252,252,248,248,240,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,224,224,224,255,255,224,224,224,255,255,255,224,224,224,255,255,224,224,224,255,255,255,224,224,224,255,255,224,224,224,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,224,240,248,192,  0,192,248,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 48, 48, 48, 48, 48,255,255,255,255,255,255,255,255, 31, 31, 31,255,255,255,255,255,255,255, 31, 31, 31,255,255,255,255,255,255, 31, 31, 31,255,255,255,255,255,255,255,255, 48, 48, 48, 48, 48,  0,  0,  0,
        0,  0,  0,  0,255,255,255,255, 31,  0,143, 63,127,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,127, 63, 31, 31,255,255,255,127,  0,192,248,252,248,240,  0,  0,  0,  0,  0, 96, 96, 96, 96, 96,255,255, 96, 96, 96, 96, 96,  0,  0,  0,  0,  0,142,142,142,142,142,255,255,255,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,255,  0,  0,  0,255,255,255,255,255,255,255,255,142,142,142,142,142,  0,  0,  0,
        0,  0,  0,  0,127,255, 31,193,252,255,255,255,254,248,  3, 15,127,127, 63,223,207,159, 63,255,255,255,255,255,255,255,255,255,255,255, 63,199,243,251,243,199,  1,240,255,255,254,248,227,131,  0,  0,223,255,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,113,113,113,113,113,255,255,255,255,255,255,255,255,254,248,240,224,195,199,207,143,143,143,  0,  0,  0,143,143,143,199,195,224,240,248,254,255,255,255,255,255,255,255,255,113,113,113,113,113,  0,  0,  0,
        0,  0,  0,  0, 63,127,255,255,255,255,255,255,143,192,252,255,254,128,192,191,255,255,254,248,227, 15, 31, 63, 63, 63, 63, 63, 15,225,252,255,255,255,255,199,248,255,255,255,255,255,255,255,248,255,255,255,255,255,127,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,240,240,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, 12, 12, 12, 12, 12,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  1,  3,  7, 15, 15, 15, 15, 31, 31, 31, 31, 31, 31, 31, 31, 31,  7,  0,  0,  0,  0,  0,  0,  0, 12, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 15, 15,  7,  7,  3,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7,  7,  7,  7,255,255,  7,  7,  7,255,255,255,  7,  7,  7,255,255,  7,  7,  7,255,255,255,  7,  7,  7,255,255,  7,  7,  7,  7,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    };

    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}
*/
#endif
