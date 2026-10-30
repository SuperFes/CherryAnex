#include QMK_KEYBOARD_H
#include "stats.h"

_Static_assert(EECONFIG_USER_DATA_SIZE >= sizeof(uint32_t), "keypress total needs the user datablock");

static keycount_t keycount;

const keycount_t *stats_keycount(void) {
    return &keycount;
}

static void save(void) {
    eeconfig_update_user_datablock(&keycount.total, 0, sizeof(keycount.total));
    keycount_mark_saved(&keycount);
}

void keyboard_post_init_user(void) {
    uint32_t stored;
    eeconfig_read_user_datablock(&stored, 0, sizeof(stored));
    keycount_init(&keycount, stored);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        keycount_press(&keycount, timer_read32());
    }
    return true;
}

void housekeeping_task_user(void) {
    if (keycount_should_save(&keycount, timer_read32())) {
        save();
    }
}

// Keeps presses since the last save when rebooting or entering the bootloader.
bool shutdown_user(bool jump_to_bootloader) {
    if (keycount.total != keycount.saved) {
        save();
    }
    return true;
}
