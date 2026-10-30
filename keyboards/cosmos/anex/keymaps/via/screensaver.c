#include "screensaver.h"

#include <string.h>

#include "storage.h"
#include "transactions.h"

#define SYNC_INTERVAL_MS 100

static screensaver_view_t view;

const screensaver_view_t *screensaver_view(void) {
    return &view;
}

static void receive_sync(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(view)) {
        memcpy(&view, in_data, sizeof(view));
    }
}

static void send_sync(void) {
    static uint32_t           last_attempt;
    static screensaver_view_t sent;
    static bool               synced;

    if ((synced && memcmp(&sent, &view, sizeof(view)) == 0) || timer_elapsed32(last_attempt) < SYNC_INTERVAL_MS) {
        return;
    }
    last_attempt = timer_read32();
    if (transaction_rpc_send(USER_SYNC_SAVER, sizeof(view), &view)) {
        sent   = view;
        synced = true;
    }
}

void screensaver_init(void) {
    uint8_t bytes[STORAGE_CHECKED_BYTES];
    eeconfig_read_user_datablock(bytes, STORAGE_SAVER_STYLE_OFFSET, sizeof(bytes));
    uint16_t style = storage_unpack(bytes);
    view.style     = style < SAVER_STYLES ? style : SAVER_STARS;
    transaction_register_rpc(USER_SYNC_SAVER, receive_sync);
}

bool screensaver_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode != SAVER_TOGGLE) {
        if (record->event.pressed) {
            view.preview = false;
        }
        return false;
    }
    if (record->event.pressed) {
        view.style   = saver_next_style(view.style);
        view.preview = true;

        uint8_t bytes[STORAGE_CHECKED_BYTES];
        storage_pack(view.style, bytes);
        eeconfig_update_user_datablock(bytes, STORAGE_SAVER_STYLE_OFFSET, sizeof(bytes));
    }
    return true;
}

void screensaver_task(void) {
    if (is_keyboard_master()) {
        send_sync();
    }
}
