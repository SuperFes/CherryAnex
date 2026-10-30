#pragma once

#include QMK_KEYBOARD_H
#include "snake.h"

// Custom keycode VIA shows as "Game" (CUSTOM(0) in via-definition.json).
#define GAME_TOGGLE QK_KB_0

bool           game_active(void);
const snake_t *game_snake(void);

// Returns true when the game consumed the key, so it must not reach the host.
bool game_record(uint16_t keycode, keyrecord_t *record);
// Returns true when the board changed and needs redrawing.
bool game_task(void);
