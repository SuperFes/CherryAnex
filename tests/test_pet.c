#include "pet.h"
#include "pet_sprite_ids.h"
#include "petfx_sprite_ids.h"
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

static void test_hop_arcs_and_lands(void) {
    pet_t pet = {.mood = PET_IDLE};
    pet_react(&pet, PET_REACT_HOP, 1000);
    CHECK(pet_animating(&pet, 1000));
    CHECK(pet_pose(&pet, 1150).dy < -3);
    CHECK(pet_pose(&pet, 1300).dy == 0);
    CHECK(!pet_animating(&pet, 1300));
}

static void test_swearing_freezes_and_outranks_hops(void) {
    pet_t pet = {.mood = PET_WALK};
    pet_react(&pet, PET_REACT_GRAWLIX, 0);
    pet_react(&pet, PET_REACT_HOP, 10);
    CHECK(pet.reaction == PET_REACT_GRAWLIX);
    pet_pose_t pose = pet_pose(&pet, 100);
    CHECK(pose.sprite == PET_SPRITE_FLUSTERED);
    CHECK(pose.fx == PETFX_SPRITE_GRAWLIX);
    CHECK(pet_update(&pet, 20, 0, 220));
    CHECK(pet.x == 0);
    // A new swear replaces the current one.
    pet_react(&pet, PET_REACT_FAINT, 300);
    CHECK(pet_pose(&pet, 400).sprite == PET_SPRITE_FAINT);
    CHECK(!pet_animating(&pet, 400));
}

static void test_reaction_expiry_redraws(void) {
    pet_t pet = {.mood = PET_IDLE};
    pet_react(&pet, PET_REACT_SOAP, 0);
    CHECK(pet_pose(&pet, 100).fx == PETFX_SPRITE_SOAP_1);
    CHECK(pet_pose(&pet, 400).fx == PETFX_SPRITE_SOAP_2);
    CHECK(pet_update(&pet, 0, 0, 2500));
    CHECK(pet.reaction == PET_REACT_NONE);
    CHECK(pet_pose(&pet, 2500).fx == -1);
}

static void test_panic_outranks_sneak(void) {
    pet_t pet = {.mood = PET_IDLE};
    pet_hold(&pet, true, true);
    pet_pose_t pose = pet_pose(&pet, 0);
    CHECK(pose.sprite == PET_SPRITE_PANIC);
    CHECK(pose.fx == PETFX_SPRITE_ALERT);
    CHECK(pet_animating(&pet, 0));
    CHECK(pet_pose(&pet, 0).dx != pet_pose(&pet, 50).dx);
}

static void test_sneak_crouches_and_creeps(void) {
    pet_t pet = {.mood = PET_WALK};
    pet_hold(&pet, true, false);
    uint8_t first = pet_pose(&pet, 0).sprite;
    CHECK(first == PET_SPRITE_CROUCH_1);
    CHECK(pet_update(&pet, 20, 0, 220));
    CHECK(pet.x == 1);
    CHECK(pet_pose(&pet, 220).sprite == PET_SPRITE_CROUCH_2);
}

static void test_sweat_drop_slides_down(void) {
    pet_t pet = {.mood = PET_IDLE};
    pet_sweat(&pet, 0);
    pet_pose_t start = pet_pose(&pet, 0);
    pet_pose_t later = pet_pose(&pet, 1000);
    CHECK(start.fx == PETFX_SPRITE_SWEAT);
    CHECK(later.fx_y > start.fx_y);
    CHECK(pet_pose(&pet, 1500).fx == -1);
}

static void test_backstep_tiptoes_back(void) {
    pet_t pet = {.mood = PET_IDLE, .x = 10};
    pet_react(&pet, PET_REACT_BACKSTEP, 0);
    pet_pose_t pose = pet_pose(&pet, 100);
    CHECK(pose.sprite == PET_SPRITE_CROUCH_1 || pose.sprite == PET_SPRITE_CROUCH_2);
    CHECK(pose.dx < 0 && pose.dx > -3);
    CHECK(pet_animating(&pet, 100));
    CHECK(pet_update(&pet, 0, 0, 200));
    CHECK(pet.x == 7);
    CHECK(pet_pose(&pet, 200).dx == 0);
}

static void test_backsteps_chain_and_alternate_feet(void) {
    pet_t pet = {.mood = PET_IDLE, .x = 10};
    pet_react(&pet, PET_REACT_BACKSTEP, 0);
    uint8_t first = pet_pose(&pet, 50).sprite;
    // Interrupted halfway: keeps the ground already covered.
    pet_react(&pet, PET_REACT_BACKSTEP, 100);
    CHECK(pet.x == 9);
    CHECK(pet_pose(&pet, 150).sprite != first);
    pet_update(&pet, 0, 0, 300);
    CHECK(pet.x == 6);
}

static void test_backstep_wraps_and_holds_walking(void) {
    pet_t pet = {.mood = PET_WALK, .x = 1};
    pet_react(&pet, PET_REACT_BACKSTEP, 100);
    // A walk step falls due mid-step-back and doesn't move it forward.
    CHECK(pet_update(&pet, 20, 0, 220));
    CHECK(pet.x == 1);
    pet_update(&pet, 20, 0, 300);
    CHECK(pet.x == PET_SCREEN_WIDTH - 2);
}

static void test_sweat_survives_more_backsteps(void) {
    pet_t pet = {.mood = PET_IDLE, .x = 50};
    pet_sweat(&pet, 0);
    pet_react(&pet, PET_REACT_BACKSTEP, 10);
    CHECK(pet_pose(&pet, 50).fx == PETFX_SPRITE_SWEAT);
}

int main(void) {
    test_mood_thresholds();
    test_mood_change_restarts_animation();
    test_waits_for_step_interval();
    test_running_moves_faster_and_wraps();
    test_idle_stays_put_and_blinks_last();
    test_timer_wraparound();
    test_every_sprite_index_is_valid();
    test_hop_arcs_and_lands();
    test_swearing_freezes_and_outranks_hops();
    test_reaction_expiry_redraws();
    test_panic_outranks_sneak();
    test_sneak_crouches_and_creeps();
    test_sweat_drop_slides_down();
    test_backstep_tiptoes_back();
    test_backsteps_chain_and_alternate_feet();
    test_backstep_wraps_and_holds_walking();
    test_sweat_survives_more_backsteps();
    return CHECK_DONE();
}
