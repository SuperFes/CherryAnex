// QMK user hooks, dispatched to the features that need them.

#include QMK_KEYBOARD_H
#include "stats.h"

void keyboard_post_init_user(void) {
    stats_init();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    stats_record(record);
    return true;
}

void housekeeping_task_user(void) {
    stats_task();
}

bool shutdown_user(bool jump_to_bootloader) {
    stats_shutdown();
    return true;
}
