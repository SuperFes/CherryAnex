#include "game.h"

#include <string.h>

#include "storage.h"
#include "transactions.h"

#define SYNC_INTERVAL_MS 100

static snake_t     snake;
static game_view_t view;
static bool        dirty;

const game_view_t *game_view(void) {
    return &view;
}

const snake_t *game_snake(void) {
    return &snake;
}

bool game_take_dirty(void) {
    bool was = dirty;
    dirty    = false;
    return was;
}

static void receive_sync(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(view)) {
        memcpy(&view, in_data, sizeof(view));
    }
}

static void send_sync(void) {
    static uint32_t    last_attempt;
    static game_view_t sent;
    static bool        synced;

    if ((synced && memcmp(&sent, &view, sizeof(view)) == 0) || timer_elapsed32(last_attempt) < SYNC_INTERVAL_MS) {
        return;
    }
    last_attempt = timer_read32();
    if (transaction_rpc_send(USER_SYNC_GAME, sizeof(view), &view)) {
        sent   = view;
        synced = true;
    }
}

static void save_best(void) {
    uint8_t bytes[SNAKE_BEST_BYTES];
    snake_best_pack(view.best, bytes);
    eeconfig_update_user_datablock(bytes, STORAGE_SNAKE_BEST_OFFSET, sizeof(bytes));
}

static void new_round(void) {
    snake_init(&snake, timer_read32());
    dirty = true;
}

void game_init(void) {
    uint8_t bytes[SNAKE_BEST_BYTES];
    eeconfig_read_user_datablock(bytes, STORAGE_SNAKE_BEST_OFFSET, sizeof(bytes));
    view.best = snake_best_unpack(bytes);
    transaction_register_rpc(USER_SYNC_GAME, receive_sync);
}

static uint8_t basic_keycode(uint16_t keycode) {
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode) || IS_QK_MODS(keycode)) {
        return keycode & 0xFF;
    }
    return keycode <= 0xFF ? keycode : KC_NO;
}

bool game_record(uint16_t keycode, keyrecord_t *record) {
    if (!view.active) {
        if (keycode == GAME_TOGGLE && record->event.pressed) {
            view.active = true;
            // Releases of keys held on entry would otherwise be swallowed, leaving them stuck on the host.
            clear_keyboard();
            new_round();
            return true;
        }
        return false;
    }
    if (!record->event.pressed) {
        return true;
    }

    uint8_t usage = basic_keycode(keycode);
    if (keycode == GAME_TOGGLE || usage == KC_ESC) {
        view.active = false;
        return true;
    }
    if (snake_can_restart(&snake, timer_read32())) {
        new_round();
    }
    snake_dir_t dir;
    if (snake_dir_for_usage(usage, &dir)) {
        snake_turn(&snake, dir, timer_read32());
    }
    return true;
}

void game_task(void) {
    if (!is_keyboard_master()) {
        return;
    }
    if (view.active && snake_update(&snake, timer_read32())) {
        dirty = true;
        if (snake.state == SNAKE_OVER && snake.score > view.best) {
            view.best = snake.score;
            save_best();
        }
    }
    view.state = snake.state;
    view.score = snake.score;
    send_sync();
}
