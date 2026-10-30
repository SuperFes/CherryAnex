#include "pc.h"

#include <string.h>

#include "hostinfo.h"
#include "transactions.h"

#define SYNC_INTERVAL_MS 250

static host_info_t info;
static pc_view_t view;

const pc_view_t *pc_view(void) {
    return &view;
}

static void receive_sync(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(view)) {
        memcpy(&view, in_data, sizeof(view));
    }
}

static void send_sync(void) {
    static uint32_t    last_attempt;
    static pc_view_t sent;
    static bool        synced;

    if ((synced && memcmp(&sent, &view, sizeof(view)) == 0) || timer_elapsed32(last_attempt) < SYNC_INTERVAL_MS) {
        return;
    }
    last_attempt = timer_read32();
    if (transaction_rpc_send(USER_SYNC_HOST, sizeof(view), &view)) {
        sent   = view;
        synced = true;
    }
}

void pc_init(void) {
    transaction_register_rpc(USER_SYNC_HOST, receive_sync);
}

bool pc_receive(uint8_t *data, uint8_t length) {
    return host_info_receive(&info, data, length, timer_read32());
}

void pc_task(void) {
    if (!is_keyboard_master()) {
        return;
    }
    view = (pc_view_t){
        .connected = host_info_fresh(&info, timer_read32()),
        .hour      = info.hour,
        .minute    = info.minute,
        .cpu       = info.cpu,
        .ram       = info.ram,
    };
    send_sync();
}
