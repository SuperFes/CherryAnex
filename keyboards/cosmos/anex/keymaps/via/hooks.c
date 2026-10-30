// QMK user hooks, dispatched to the features that need them.

#include QMK_KEYBOARD_H
#include "game.h"
#include "pc.h"
#include "reactions.h"
#include "screensaver.h"
#include "stats.h"

void keyboard_post_init_user(void) {
    stats_init();
    game_init();
    pc_init();
    screensaver_init();
    reactions_init();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    stats_record(record);
    // The screensaver sees every key first, so any press ends its preview.
    if (screensaver_record(keycode, record) || game_record(keycode, record)) {
        return false;
    }
    reactions_record(keycode, record);
    return true;
}

void housekeeping_task_user(void) {
    stats_task();
    game_task();
    pc_task();
    screensaver_task();
    reactions_task();
}

bool shutdown_user(bool jump_to_bootloader) {
    stats_shutdown();
    return true;
}

// Runs before VIA's own handling of each raw HID report.
bool via_command_kb(uint8_t *data, uint8_t length) {
    return pc_receive(data, length);
}
