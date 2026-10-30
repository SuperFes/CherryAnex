#pragma once

#include QMK_KEYBOARD_H
#include "snake.h"

// Custom keycode VIA shows as "Game" (CUSTOM(0) in via-definition.json).
#define GAME_TOGGLE QK_KB_0

// What either half needs to show about the game; synced to the non-USB half.
typedef struct {
    bool     active;
    uint8_t  state; // snake_state_t
    uint16_t score;
    uint16_t best;
} game_view_t;

const game_view_t *game_view(void);
const snake_t     *game_snake(void);

void game_init(void);
// Returns true when the game consumed the key, so it must not reach the host.
bool game_record(uint16_t keycode, keyrecord_t *record);
void game_task(void);
// Returns true once after the board changes, for redrawing.
bool game_take_dirty(void);
