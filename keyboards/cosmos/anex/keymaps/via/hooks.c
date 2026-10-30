// QMK user hooks, dispatched to the features that need them.

#include QMK_KEYBOARD_H
#include "game.h"
#include "pc.h"
#include "stats.h"

void keyboard_post_init_user(void) {
    stats_init();
    game_init();
    pc_init();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    stats_record(record);
    return !game_record(keycode, record);
}

void housekeeping_task_user(void) {
    stats_task();
    game_task();
    pc_task();
}

bool shutdown_user(bool jump_to_bootloader) {
    stats_shutdown();
    return true;
}

// Runs before VIA's own handling of each raw HID report.
bool via_command_kb(uint8_t *data, uint8_t length) {
    return pc_receive(data, length);
}
