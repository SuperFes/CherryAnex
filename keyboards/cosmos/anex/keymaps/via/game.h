#pragma once

#include QMK_KEYBOARD_H
#include "runner.h"
#include "snake.h"

// Custom keycode VIA shows as "Game" (CUSTOM(0) in via-definition.json); opens the game menu.
#define GAME_TOGGLE QK_KB_0

typedef enum {
    GAME_SNAKE,
    GAME_RUNNER,
    GAME_COUNT,
} game_kind_t;

// What either half needs to show about the games; synced to the non-USB half.
typedef struct {
    bool     active;
    bool     menu;  // choosing a game; kind is the highlighted one
    uint8_t  kind;  // game_kind_t
    uint8_t  state; // snake_state_t or runner_state_t; READY, PLAYING, OVER in both
    uint16_t score;
    uint16_t best;
} game_view_t;

const game_view_t *game_view(void);
const snake_t     *game_snake(void);
const runner_t    *game_runner(void);
const char        *game_name(game_kind_t kind);

void game_init(void);
// Returns true when the game consumed the key, so it must not reach the host.
bool game_record(uint16_t keycode, keyrecord_t *record);
void game_task(void);
// Returns true once after the screen changes, for redrawing.
bool game_take_dirty(void);
