#include "pet.h"
#include "pet_sprite_ids.h"

typedef struct {
    uint8_t  sprites[PET_STEPS];
    uint16_t interval_ms;
    uint8_t  speed; // pixels moved per step
} pet_mood_def_t;

static const pet_mood_def_t moods[] = {
    [PET_SLEEP] = {{PET_SPRITE_SLEEP_1, PET_SPRITE_SLEEP_1, PET_SPRITE_SLEEP_2, PET_SPRITE_SLEEP_2}, 700, 0},
    [PET_IDLE]  = {{PET_SPRITE_IDLE, PET_SPRITE_IDLE, PET_SPRITE_IDLE, PET_SPRITE_BLINK}, 500, 0},
    [PET_WALK]  = {{PET_SPRITE_WALK_1, PET_SPRITE_WALK_2, PET_SPRITE_WALK_1, PET_SPRITE_WALK_2}, 220, 2},
    [PET_RUN]   = {{PET_SPRITE_RUN_1, PET_SPRITE_RUN_2, PET_SPRITE_RUN_1, PET_SPRITE_RUN_2}, 100, 4},
};

pet_mood_t pet_mood_for(uint8_t wpm, uint32_t idle_ms) {
    if (wpm >= PET_RUN_WPM) {
        return PET_RUN;
    }
    if (wpm > 0) {
        return PET_WALK;
    }
    if (idle_ms >= PET_SLEEP_AFTER_MS) {
        return PET_SLEEP;
    }
    return PET_IDLE;
}

bool pet_update(pet_t *pet, uint8_t wpm, uint32_t idle_ms, uint32_t now) {
    pet_mood_t mood = pet_mood_for(wpm, idle_ms);

    if (mood != pet->mood) {
        pet->mood      = mood;
        pet->step      = 0;
        pet->last_step = now;
        return true;
    }
    const pet_mood_def_t *def = &moods[mood];
    if (now - pet->last_step < def->interval_ms) {
        return false;
    }
    pet->step      = (pet->step + 1) % PET_STEPS;
    pet->x         = (uint8_t)((pet->x + def->speed) % PET_SCREEN_WIDTH);
    pet->last_step = now;
    return true;
}

uint8_t pet_sprite(const pet_t *pet) {
    return moods[pet->mood].sprites[pet->step];
}
