#include "game.h"

#include <string.h>

#include "storage.h"
#include "transactions.h"

#define SYNC_INTERVAL_MS 100

_Static_assert((int)SNAKE_READY == (int)RUNNER_READY && (int)SNAKE_PLAYING == (int)RUNNER_PLAYING && (int)SNAKE_OVER == (int)RUNNER_OVER, "game_view_t.state is shared");

static const uint8_t best_offsets[GAME_COUNT] = {
    [GAME_SNAKE]  = STORAGE_SNAKE_BEST_OFFSET,
    [GAME_RUNNER] = STORAGE_RUNNER_BEST_OFFSET,
};

static snake_t     snake;
static runner_t    runner;
static uint16_t    best[GAME_COUNT];
static game_view_t view;
static bool        dirty;

const game_view_t *game_view(void) {
    return &view;
}

const snake_t *game_snake(void) {
    return &snake;
}

const runner_t *game_runner(void) {
    return &runner;
}

const char *game_name(game_kind_t kind) {
    return kind == GAME_RUNNER ? "RUNNER" : "SNAKE";
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

static void save_best(game_kind_t kind) {
    uint8_t bytes[STORAGE_CHECKED_BYTES];
    storage_pack(best[kind], bytes);
    eeconfig_update_user_datablock(bytes, best_offsets[kind], sizeof(bytes));
}

void game_init(void) {
    for (uint8_t kind = 0; kind < GAME_COUNT; ++kind) {
        uint8_t bytes[STORAGE_CHECKED_BYTES];
        eeconfig_read_user_datablock(bytes, best_offsets[kind], sizeof(bytes));
        best[kind] = storage_unpack(bytes);
    }
    transaction_register_rpc(USER_SYNC_GAME, receive_sync);
}

static void new_round(void) {
    if (view.kind == GAME_RUNNER) {
        runner_init(&runner, timer_read32());
    } else {
        snake_init(&snake, timer_read32());
    }
    dirty = true;
}

static uint8_t basic_keycode(uint16_t keycode) {
    if (IS_QK_MOD_TAP(keycode) || IS_QK_LAYER_TAP(keycode) || IS_QK_MODS(keycode)) {
        return keycode & 0xFF;
    }
    return keycode <= 0xFF ? keycode : KC_NO;
}

static void menu_key(uint8_t usage) {
    snake_dir_t dir;
    bool        is_dir = snake_dir_for_usage(usage, &dir);

    if (is_dir && (dir == SNAKE_UP || dir == SNAKE_DOWN)) {
        view.kind = (view.kind + (dir == SNAKE_DOWN ? 1 : GAME_COUNT - 1)) % GAME_COUNT;
        dirty     = true;
    } else if (usage == KC_ENTER || usage == KC_SPACE || (is_dir && dir == SNAKE_RIGHT)) {
        view.menu = false;
        new_round();
    }
}

static void snake_key(uint8_t usage, uint32_t now) {
    if (snake_can_restart(&snake, now)) {
        new_round();
    }
    snake_dir_t dir;
    if (snake_dir_for_usage(usage, &dir)) {
        snake_turn(&snake, dir, now);
    }
}

static void runner_key(uint8_t usage, uint32_t now) {
    if (runner_can_restart(&runner, now)) {
        new_round();
        return;
    }
    snake_dir_t dir;
    bool        is_dir = snake_dir_for_usage(usage, &dir);
    if (usage == KC_SPACE || (is_dir && dir == SNAKE_UP)) {
        runner_jump(&runner, now);
    } else if (is_dir && dir == SNAKE_DOWN) {
        runner_drop(&runner);
    }
}

bool game_record(uint16_t keycode, keyrecord_t *record) {
    if (!view.active) {
        if (keycode == GAME_TOGGLE && record->event.pressed) {
            view.active = true;
            view.menu   = true;
            dirty       = true;
            // Releases of keys held on entry would otherwise be swallowed, leaving them stuck on the host.
            clear_keyboard();
            return true;
        }
        return false;
    }
    if (!record->event.pressed) {
        return true;
    }

    uint8_t  usage = basic_keycode(keycode);
    uint32_t now   = timer_read32();
    if (keycode == GAME_TOGGLE || (usage == KC_ESC && view.menu)) {
        view.active = false;
    } else if (usage == KC_ESC) {
        view.menu = true;
        dirty     = true;
    } else if (view.menu) {
        menu_key(usage);
    } else if (view.kind == GAME_RUNNER) {
        runner_key(usage, now);
    } else {
        snake_key(usage, now);
    }
    return true;
}

static void update_score(uint8_t state, uint16_t score) {
    view.state = state;
    view.score = score;
    if (state == SNAKE_OVER && score > best[view.kind]) {
        best[view.kind] = score;
        save_best(view.kind);
    }
}

void game_task(void) {
    if (!is_keyboard_master()) {
        return;
    }
    if (view.active && !view.menu) {
        uint32_t now = timer_read32();
        if (view.kind == GAME_RUNNER) {
            dirty |= runner_update(&runner, now);
            update_score(runner.state, runner.score);
        } else {
            dirty |= snake_update(&snake, now);
            update_score(snake.state, snake.score);
        }
    } else {
        view.state = SNAKE_READY;
        view.score = 0;
    }
    view.best = best[view.kind];
    send_sync();
}
