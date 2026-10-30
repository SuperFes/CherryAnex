#include "pet.h"
#include "pet_sprite_ids.h"
#include "check.h"

static void test_mood_thresholds(void) {
    CHECK(pet_mood_for(0, 0) == PET_IDLE);
    CHECK(pet_mood_for(0, PET_SLEEP_AFTER_MS - 1) == PET_IDLE);
    CHECK(pet_mood_for(0, PET_SLEEP_AFTER_MS) == PET_SLEEP);
    CHECK(pet_mood_for(1, PET_SLEEP_AFTER_MS) == PET_WALK);
    CHECK(pet_mood_for(PET_RUN_WPM - 1, 0) == PET_WALK);
    CHECK(pet_mood_for(PET_RUN_WPM, 0) == PET_RUN);
}

static void test_mood_change_restarts_animation(void) {
    pet_t pet = {.mood = PET_IDLE, .step = 2, .x = 10, .last_step = 0};
    CHECK(pet_update(&pet, 20, 0, 50));
    CHECK(pet.mood == PET_WALK);
    CHECK(pet.step == 0);
    CHECK(pet.x == 10);
    CHECK(pet.last_step == 50);
}

static void test_waits_for_step_interval(void) {
    pet_t pet = {.mood = PET_WALK};
    CHECK(!pet_update(&pet, 20, 0, 219));
    CHECK(pet.step == 0);
    CHECK(pet_update(&pet, 20, 0, 220));
    CHECK(pet.step == 1);
    CHECK(pet.x == 2);
}

static void test_running_moves_faster_and_wraps(void) {
    pet_t pet = {.mood = PET_RUN, .x = PET_SCREEN_WIDTH - 2};
    CHECK(pet_update(&pet, 80, 0, 100));
    CHECK(pet.x == 2);
}

static void test_idle_stays_put_and_blinks_last(void) {
    pet_t   pet = {.mood = PET_IDLE};
    uint8_t sprites[PET_STEPS];
    for (uint8_t i = 0; i < PET_STEPS; ++i) {
        sprites[i] = pet_sprite(&pet);
        CHECK(pet_update(&pet, 0, 0, (i + 1) * 500u));
    }
    CHECK(pet.x == 0);
    CHECK(sprites[0] == PET_SPRITE_IDLE);
    CHECK(sprites[PET_STEPS - 1] == PET_SPRITE_BLINK);
    CHECK(pet.step == 0);
}

static void test_timer_wraparound(void) {
    pet_t pet = {.mood = PET_RUN, .last_step = 0xFFFFFFF0u};
    CHECK(pet_update(&pet, 80, 0, 0x60));
    CHECK(pet.step == 1);
}

static void test_every_sprite_index_is_valid(void) {
    for (pet_mood_t mood = PET_SLEEP; mood <= PET_RUN; ++mood) {
        for (uint8_t step = 0; step < PET_STEPS; ++step) {
            pet_t pet = {.mood = mood, .step = step};
            CHECK(pet_sprite(&pet) < PET_SPRITE_COUNT);
        }
    }
}

int main(void) {
    test_mood_thresholds();
    test_mood_change_restarts_animation();
    test_waits_for_step_interval();
    test_running_moves_faster_and_wraps();
    test_idle_stays_put_and_blinks_last();
    test_timer_wraparound();
    test_every_sprite_index_is_valid();
    return CHECK_DONE();
}
