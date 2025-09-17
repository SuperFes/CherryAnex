#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_ESCAPE,   KC_F1,   KC_F2,   KC_F3,   KC_F4,    KC_F5,   KC_F6,     /* <-L R-> */ KC_F7,    KC_F8,    KC_F9,     KC_F10,    KC_F11,      KC_F12,    KC_BACKSPACE, \
                 KC_TILDE,    KC_1,    KC_2,    KC_3,    KC_4,     KC_5,    KC_6,      /* <-L R-> */ KC_7,     KC_8,     KC_9,      KC_0,      KC_MINUS,    KC_PLUS,   KC_INSERT, \
                 KC_TAB,      KC_Q,    KC_W,    KC_E,    KC_R,     KC_T,    KC_SPACE,  /* <-L R-> */ KC_SPACE, KC_Y,     KC_U,      KC_I,      KC_O,        KC_P,      KC_DEL, \
                 KC_CAPSLOCK, KC_A,    KC_S,    KC_D,    KC_F,     KC_G,    KC_SPACE,  /* <-L R-> */ KC_SPACE, KC_H,     KC_J,      KC_K,      KC_L,        KC_COLON,  KC_QUOTE, \
                 KC_ALT,      KC_Z,    KC_X,    KC_C,    KC_V,     KC_B,    KC_SPACE,  /* <-L R-> */ KC_SPACE, KC_N,     KC_M,      KC_COMMA,  KC_PERIOD,   KC_FSLASH, KC_UP,
                 KC_CTRL,     KC_META, KC_LSQB, KC_RSQB, KC_SPACE, KC_LEFT, KC_FSLASH, /* <-L R-> */ KC_TILDE, KC_FN,    KC_SPACE,  KC_ESCAPE, KC_PRINTSCR, KC_HOME,   KC_END),
};

void keyboard_post_init_user(void) {
    debug_enable=true;
    debug_matrix=true;
}
