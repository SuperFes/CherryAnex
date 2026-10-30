#pragma once

// Endless runner: the chip pet jumps over components scrolling in from the
// right of a 128x64 display. Distances and speeds are in 1/16 px.
// Free of QMK headers so it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"
#include "runner_sprite_ids.h"

#define RUNNER_FP 16
#define RUNNER_TICK_MS 30
#define RUNNER_GROUND_Y 60
#define RUNNER_PET_X 4
#define RUNNER_PET_SIZE 32
#define RUNNER_START_SPEED 36
#define RUNNER_MAX_SPEED 64
#define RUNNER_JUMP_VELOCITY 72
#define RUNNER_GRAVITY 6
#define RUNNER_DROP_VELOCITY 64
#define RUNNER_MAX_OBSTACLES 4
// Keeps a key mashed at the moment of death from starting the next round.
#define RUNNER_RESTART_DELAY_MS 600

typedef enum {
    RUNNER_READY, // waiting for the first jump
    RUNNER_PLAYING,
    RUNNER_OVER,
} runner_state_t;

typedef struct {
    bool    used;
    uint8_t type; // RUNNER_SPRITE_*
    int16_t x;    // left edge of the sprite
} runner_obstacle_t;

typedef struct {
    runner_state_t    state;
    int16_t           lift; // pet's height above the ground
    int16_t           velocity;
    uint32_t          distance;
    uint16_t          score;
    uint16_t          speed;
    int32_t           until_spawn;
    runner_obstacle_t obstacles[RUNNER_MAX_OBSTACLES];
    uint32_t          last_tick;
    uint32_t          over_at;
    uint32_t          rng;
} runner_t;

typedef struct {
    int16_t left;
    int16_t right; // exclusive
    int16_t top;
    int16_t bottom; // inclusive
} runner_box_t;

void runner_init(runner_t *runner, uint32_t seed);

// Starts the round from RUNNER_READY. Ignored in the air.
void runner_jump(runner_t *runner, uint32_t now);
// Falls fast while in the air.
void runner_drop(runner_t *runner);

// Runs any ticks due by now. Returns true on change.
bool runner_update(runner_t *runner, uint32_t now);
void runner_step(runner_t *runner);

bool         runner_can_restart(const runner_t *runner, uint32_t now);
bool         runner_airborne(const runner_t *runner);
runner_box_t runner_pet_box(const runner_t *runner);
runner_box_t runner_obstacle_box(const runner_obstacle_t *obstacle);

// pet_frames: run_1, run_2 and the frame shown after a crash, each
// RUNNER_PET_SIZE square. obstacle_sprites: indexed by RUNNER_SPRITE_*.
void runner_draw(canvas_t *canvas, const runner_t *runner, const uint8_t *const pet_frames[3], const uint8_t *const obstacle_sprites[RUNNER_SPRITE_COUNT]);
