#include "runner.h"
#include "rng.h"

#include <string.h>

// Ticks run at once after a stall, so a slow frame doesn't freeze the game.
#define MAX_CATCH_UP_TICKS 4
// Pixels of travel per point.
#define SCORE_PX 8
// Points per 1/16 px/tick of extra speed.
#define SPEEDUP_POINTS 12
// Clear ground before the first obstacle, in px.
#define FIRST_GAP_PX 96
// Room past a full jump's landing, in px, and the random spread on top.
#define GAP_MARGIN_PX 24
#define GAP_SPREAD_PX 64

// Collision boxes within the 16x16 obstacle sprites; see art/runner.txt.
static const struct {
    uint8_t left;
    uint8_t width;
    uint8_t height;
} obstacle_boxes[RUNNER_SPRITE_COUNT] = {
    [RUNNER_SPRITE_CHIP]     = {1, 13, 8},
    [RUNNER_SPRITE_CAP]      = {5, 6, 14},
    [RUNNER_SPRITE_RESISTOR] = {2, 12, 9},
};

// The pet's body within its 32x32 sprite, a little inside the art to be fair.
#define PET_BOX_LEFT 8
#define PET_BOX_WIDTH 17
#define PET_BOX_TOP 10
#define PET_BOX_BOTTOM 27

void runner_init(runner_t *runner, uint32_t seed) {
    memset(runner, 0, sizeof(*runner));
    runner->rng         = seed ? seed : 1;
    runner->speed       = RUNNER_START_SPEED;
    runner->until_spawn = FIRST_GAP_PX * RUNNER_FP;
}

bool runner_airborne(const runner_t *runner) {
    return runner->lift > 0 || runner->velocity > 0;
}

void runner_jump(runner_t *runner, uint32_t now) {
    if (runner->state == RUNNER_OVER) {
        return;
    }
    if (runner->state == RUNNER_READY) {
        runner->state     = RUNNER_PLAYING;
        runner->last_tick = now;
    }
    if (!runner_airborne(runner)) {
        runner->velocity = RUNNER_JUMP_VELOCITY;
    }
}

void runner_drop(runner_t *runner) {
    if (runner->state == RUNNER_PLAYING && runner_airborne(runner) && runner->velocity > -RUNNER_DROP_VELOCITY) {
        runner->velocity = -RUNNER_DROP_VELOCITY;
    }
}

bool runner_can_restart(const runner_t *runner, uint32_t now) {
    return runner->state == RUNNER_OVER && now - runner->over_at >= RUNNER_RESTART_DELAY_MS;
}

runner_box_t runner_pet_box(const runner_t *runner) {
    int16_t bottom = RUNNER_GROUND_Y - runner->lift / RUNNER_FP;
    return (runner_box_t){
        .left   = RUNNER_PET_X + PET_BOX_LEFT,
        .right  = RUNNER_PET_X + PET_BOX_LEFT + PET_BOX_WIDTH,
        .top    = bottom - (PET_BOX_BOTTOM - PET_BOX_TOP),
        .bottom = bottom,
    };
}

runner_box_t runner_obstacle_box(const runner_obstacle_t *obstacle) {
    int16_t left = obstacle->x / RUNNER_FP + obstacle_boxes[obstacle->type].left;
    return (runner_box_t){
        .left   = left,
        .right  = left + obstacle_boxes[obstacle->type].width,
        .top    = RUNNER_GROUND_Y - obstacle_boxes[obstacle->type].height + 1,
        .bottom = RUNNER_GROUND_Y,
    };
}

static bool overlap(runner_box_t a, runner_box_t b) {
    return a.left < b.right && b.left < a.right && a.top <= b.bottom && b.top <= a.bottom;
}

// Ground covered by one full jump at this speed, so gaps can always be cleared.
static int32_t jump_distance(uint16_t speed) {
    int32_t air_ticks = 2 * RUNNER_JUMP_VELOCITY / RUNNER_GRAVITY + 1;
    return air_ticks * speed;
}

static void spawn(runner_t *runner) {
    for (uint8_t i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
        runner_obstacle_t *obstacle = &runner->obstacles[i];
        if (!obstacle->used) {
            obstacle->used = true;
            obstacle->type = rng_next(&runner->rng) % RUNNER_SPRITE_COUNT;
            obstacle->x    = CANVAS_WIDTH * RUNNER_FP;
            break;
        }
    }
    runner->until_spawn = jump_distance(runner->speed) + (GAP_MARGIN_PX + rng_next(&runner->rng) % GAP_SPREAD_PX) * RUNNER_FP;
}

void runner_step(runner_t *runner) {
    runner->distance += runner->speed;
    runner->score = runner->distance / (SCORE_PX * RUNNER_FP) > UINT16_MAX ? UINT16_MAX : runner->distance / (SCORE_PX * RUNNER_FP);
    uint16_t speed = RUNNER_START_SPEED + runner->score / SPEEDUP_POINTS;
    runner->speed  = speed > RUNNER_MAX_SPEED ? RUNNER_MAX_SPEED : speed;

    if (runner_airborne(runner)) {
        runner->lift += runner->velocity;
        runner->velocity -= RUNNER_GRAVITY;
        if (runner->lift <= 0) {
            runner->lift     = 0;
            runner->velocity = 0;
        }
    }

    for (uint8_t i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
        runner_obstacle_t *obstacle = &runner->obstacles[i];
        if (!obstacle->used) {
            continue;
        }
        obstacle->x -= runner->speed;
        if (obstacle->x < -RUNNER_SPRITE_WIDTH * RUNNER_FP) {
            obstacle->used = false;
        }
    }

    runner->until_spawn -= runner->speed;
    if (runner->until_spawn <= 0) {
        spawn(runner);
    }

    runner_box_t pet = runner_pet_box(runner);
    for (uint8_t i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
        if (runner->obstacles[i].used && overlap(pet, runner_obstacle_box(&runner->obstacles[i]))) {
            runner->state = RUNNER_OVER;
            return;
        }
    }
}

bool runner_update(runner_t *runner, uint32_t now) {
    if (runner->state != RUNNER_PLAYING) {
        return false;
    }
    uint32_t due = (now - runner->last_tick) / RUNNER_TICK_MS;
    if (!due) {
        return false;
    }
    if (due > MAX_CATCH_UP_TICKS) {
        due               = MAX_CATCH_UP_TICKS;
        runner->last_tick = now;
    } else {
        runner->last_tick += due * RUNNER_TICK_MS;
    }
    while (due-- && runner->state == RUNNER_PLAYING) {
        runner_step(runner);
    }
    if (runner->state == RUNNER_OVER) {
        runner->over_at = now;
    }
    return true;
}

void runner_draw(canvas_t *canvas, const runner_t *runner, const uint8_t *const pet_frames[3], const uint8_t *const obstacle_sprites[RUNNER_SPRITE_COUNT]) {
    canvas_clear(canvas);

    uint32_t travelled = runner->distance / RUNNER_FP;
    for (int16_t x = 0; x < CANVAS_WIDTH; ++x) {
        canvas_pixel(canvas, x, RUNNER_GROUND_Y);
        // Pebbles scroll with the ground so speed is visible.
        if ((x + travelled) % 9 == 0) {
            canvas_pixel(canvas, x, RUNNER_GROUND_Y + 2);
        }
        if ((x + travelled) % 13 == 5) {
            canvas_pixel(canvas, x, RUNNER_GROUND_Y + 3);
        }
    }

    for (uint8_t i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
        const runner_obstacle_t *obstacle = &runner->obstacles[i];
        if (obstacle->used) {
            canvas_blit(canvas, obstacle_sprites[obstacle->type], RUNNER_SPRITE_WIDTH, RUNNER_SPRITE_HEIGHT, obstacle->x / RUNNER_FP, RUNNER_GROUND_Y - RUNNER_SPRITE_HEIGHT + 1);
        }
    }

    uint8_t frame;
    if (runner->state == RUNNER_OVER) {
        frame = 2;
    } else if (runner_airborne(runner) || runner->state == RUNNER_READY) {
        frame = 0;
    } else {
        frame = (travelled / 10) % 2;
    }
    canvas_blit(canvas, pet_frames[frame], RUNNER_PET_SIZE, RUNNER_PET_SIZE, RUNNER_PET_X, RUNNER_GROUND_Y - PET_BOX_BOTTOM - runner->lift / RUNNER_FP);
}
