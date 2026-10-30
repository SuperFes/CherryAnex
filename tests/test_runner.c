#include "runner.h"
#include "check.h"

static canvas_t canvas;

static int lit(int16_t x, int16_t y) {
    return (canvas.bytes[(y / 8) * CANVAS_WIDTH + x] >> (y % 8)) & 1;
}

static void started(runner_t *r) {
    runner_init(r, 99);
    r->state = RUNNER_PLAYING;
}

static void test_first_jump_starts_round(void) {
    runner_t r;
    runner_init(&r, 99);
    CHECK(r.state == RUNNER_READY);
    CHECK(!runner_update(&r, 10000));
    runner_jump(&r, 500);
    CHECK(r.state == RUNNER_PLAYING);
    CHECK(!runner_update(&r, 500 + RUNNER_TICK_MS - 1));
    CHECK(runner_update(&r, 500 + RUNNER_TICK_MS));
    CHECK(r.lift > 0);
}

static void test_jump_lands_and_cannot_double_jump(void) {
    runner_t r;
    started(&r);
    runner_jump(&r, 0);
    int16_t peak  = 0;
    int     ticks = 0;
    do {
        runner_step(&r);
        if (r.lift > peak) {
            peak = r.lift;
        }
        if (ticks == 5) {
            int16_t velocity = r.velocity;
            runner_jump(&r, 0);
            CHECK(r.velocity == velocity);
        }
        ++ticks;
    } while (runner_airborne(&r) && ticks < 100);
    CHECK(r.lift == 0 && r.velocity == 0);
    CHECK(ticks >= 2 * RUNNER_JUMP_VELOCITY / RUNNER_GRAVITY - 1 && ticks <= 2 * RUNNER_JUMP_VELOCITY / RUNNER_GRAVITY + 2);
    // Higher than the tallest obstacle, with room for the pet's feet.
    CHECK(peak / RUNNER_FP > 20);
}

static void test_drop_lands_sooner(void) {
    runner_t r;
    started(&r);
    runner_jump(&r, 0);
    for (int i = 0; i < 4; ++i) {
        runner_step(&r);
    }
    runner_drop(&r);
    int ticks = 0;
    while (runner_airborne(&r) && ticks < 100) {
        runner_step(&r);
        ++ticks;
    }
    CHECK(ticks < 8);
}

static void test_obstacles_spawn_scroll_and_expire(void) {
    runner_t r;
    started(&r);
    int i = 0;
    while (!r.obstacles[0].used && i++ < 1000) {
        r.lift = 0;
        runner_step(&r);
    }
    CHECK(r.obstacles[0].used);
    int16_t x = r.obstacles[0].x;
    // Park the pet high above everything so nothing collides.
    r.lift     = 40 * RUNNER_FP;
    r.velocity = 0;
    runner_step(&r);
    CHECK(r.obstacles[0].x == x - r.speed);
    for (i = 0; i < 400 && r.obstacles[0].used && r.obstacles[0].x <= x; ++i) {
        r.lift     = 40 * RUNNER_FP;
        r.velocity = 0;
        runner_step(&r);
    }
    CHECK(!r.obstacles[0].used || r.obstacles[0].x > x);
}

static void test_collision_ends_round(void) {
    runner_t r;
    started(&r);
    r.until_spawn  = 1 << 30;
    r.obstacles[0] = (runner_obstacle_t){.used = true, .type = RUNNER_SPRITE_CAP, .x = (RUNNER_PET_X + 10) * RUNNER_FP};
    runner_step(&r);
    CHECK(r.state == RUNNER_OVER);

    started(&r);
    r.until_spawn  = 1 << 30;
    r.obstacles[0] = (runner_obstacle_t){.used = true, .type = RUNNER_SPRITE_CAP, .x = (RUNNER_PET_X + 10) * RUNNER_FP};
    r.lift         = 20 * RUNNER_FP;
    r.velocity     = RUNNER_GRAVITY;
    runner_step(&r);
    CHECK(r.state == RUNNER_PLAYING);
}

static void test_speeds_up_to_a_limit(void) {
    runner_t r;
    started(&r);
    r.until_spawn = 1 << 30;
    CHECK(r.speed == RUNNER_START_SPEED);
    r.distance = 200000u * RUNNER_FP;
    runner_step(&r);
    CHECK(r.speed == RUNNER_MAX_SPEED);
    CHECK(r.score > 0);
}

// A bot that jumps when an obstacle gets close must survive a long run, so
// every gap the spawner makes is clearable.
static void test_gaps_are_always_clearable(void) {
    for (uint32_t seed = 1; seed <= 20; ++seed) {
        runner_t r;
        runner_init(&r, seed);
        runner_jump(&r, 0);
        for (int tick = 0; tick < 6000 && r.state == RUNNER_PLAYING; ++tick) {
            runner_box_t pet = runner_pet_box(&r);
            for (uint8_t i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
                if (!r.obstacles[i].used) {
                    continue;
                }
                int16_t ahead = runner_obstacle_box(&r.obstacles[i]).left - pet.right;
                if (ahead >= 0 && ahead <= r.speed * 5 / RUNNER_FP) {
                    runner_jump(&r, 0);
                }
            }
            runner_step(&r);
        }
        CHECK(r.state == RUNNER_PLAYING);
        CHECK(r.speed == RUNNER_MAX_SPEED);
    }
}

static void test_restart_delay(void) {
    runner_t r;
    started(&r);
    r.last_tick    = 0;
    r.until_spawn  = 1 << 30;
    r.obstacles[0] = (runner_obstacle_t){.used = true, .type = RUNNER_SPRITE_CHIP, .x = (RUNNER_PET_X + 10) * RUNNER_FP};
    CHECK(runner_update(&r, RUNNER_TICK_MS));
    CHECK(r.state == RUNNER_OVER);
    CHECK(!runner_can_restart(&r, RUNNER_TICK_MS + RUNNER_RESTART_DELAY_MS - 1));
    CHECK(runner_can_restart(&r, RUNNER_TICK_MS + RUNNER_RESTART_DELAY_MS));
    runner_jump(&r, 0);
    CHECK(r.state == RUNNER_OVER);
}

static void test_draw_places_pet_on_ground(void) {
    static uint8_t        solid[RUNNER_PET_SIZE * RUNNER_PET_SIZE / 8];
    static uint8_t        block[RUNNER_SPRITE_WIDTH * RUNNER_SPRITE_HEIGHT / 8];
    const uint8_t *const  pet[3]                        = {solid, solid, solid};
    const uint8_t        *obstacles[RUNNER_SPRITE_COUNT] = {block, block, block};
    for (unsigned i = 0; i < sizeof(solid); ++i) {
        solid[i] = 0xFF;
    }
    runner_t r;
    runner_init(&r, 1);
    runner_draw(&canvas, &r, pet, obstacles);
    CHECK(lit(RUNNER_PET_X, RUNNER_GROUND_Y - 27));
    CHECK(!lit(RUNNER_PET_X, RUNNER_GROUND_Y - 28));
    CHECK(lit(CANVAS_WIDTH - 1, RUNNER_GROUND_Y));
}

int main(void) {
    test_first_jump_starts_round();
    test_jump_lands_and_cannot_double_jump();
    test_drop_lands_sooner();
    test_obstacles_spawn_scroll_and_expire();
    test_collision_ends_round();
    test_speeds_up_to_a_limit();
    test_gaps_are_always_clearable();
    test_restart_delay();
    test_draw_places_pet_on_ground();
    return CHECK_DONE();
}
