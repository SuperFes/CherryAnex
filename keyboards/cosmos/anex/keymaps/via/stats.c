#include QMK_KEYBOARD_H
#include "stats.h"
#include "storage.h"
#include "transactions.h"

_Static_assert(EECONFIG_USER_DATA_SIZE >= STORAGE_SIZE, "storage.h layout exceeds the user datablock");

#define SYNC_INTERVAL_MS 500

typedef struct {
    uint32_t total;
    uint32_t session;
} keycount_sync_t;

static keycount_t keycount;

const keycount_t *stats_keycount(void) {
    return &keycount;
}

static void save(void) {
    eeconfig_update_user_datablock(&keycount.total, STORAGE_KEYCOUNT_OFFSET, sizeof(keycount.total));
    keycount_mark_saved(&keycount);
}

// Only the USB half sees keypresses; the other half just displays its counts.
static void receive_sync(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(keycount_sync_t)) {
        const keycount_sync_t *sync = in_data;
        keycount.total              = sync->total;
        keycount.session            = sync->session;
    }
}

static void send_sync(void) {
    static uint32_t last_attempt;
    static uint32_t sent_session = UINT32_MAX;

    if (keycount.session == sent_session || timer_elapsed32(last_attempt) < SYNC_INTERVAL_MS) {
        return;
    }
    last_attempt         = timer_read32();
    keycount_sync_t sync = {.total = keycount.total, .session = keycount.session};
    if (transaction_rpc_send(USER_SYNC_KEYCOUNT, sizeof(sync), &sync)) {
        sent_session = sync.session;
    }
}

void stats_init(void) {
    uint32_t stored;
    eeconfig_read_user_datablock(&stored, STORAGE_KEYCOUNT_OFFSET, sizeof(stored));
    keycount_init(&keycount, stored);
    transaction_register_rpc(USER_SYNC_KEYCOUNT, receive_sync);
}

void stats_record(keyrecord_t *record) {
    if (record->event.pressed) {
        keycount_press(&keycount, timer_read32());
    }
}

void stats_task(void) {
    if (!is_keyboard_master()) {
        return;
    }
    if (keycount_should_save(&keycount, timer_read32())) {
        save();
    }
    send_sync();
}

// Keeps presses since the last save when rebooting or entering the bootloader.
void stats_shutdown(void) {
    if (is_keyboard_master() && keycount.total != keycount.saved) {
        save();
    }
}
