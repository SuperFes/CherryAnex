#include "pet.h"
#include "pet_sprite_ids.h"
#include "petfx_sprite_ids.h"

static const uint16_t reaction_ms[PET_REACTIONS] = {
    [PET_REACT_HOP]     = 300,
    [PET_REACT_SWEAT]   = 1500,
    [PET_REACT_GRAWLIX] = 2500,
    [PET_REACT_SOAP]    = 2500,
    [PET_REACT_FAINT]   = 2500,
};
#define HOP_HEIGHT 6

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

static bool reacting(const pet_t *pet, uint32_t now) {
    return pet->reaction != PET_REACT_NONE && now - pet->reaction_at < reaction_ms[pet->reaction];
}

static bool swearing(const pet_t *pet, uint32_t now) {
    return reacting(pet, now) && pet->reaction >= PET_FIRST_SWEAR_REACTION;
}

bool pet_update(pet_t *pet, uint8_t wpm, uint32_t idle_ms, uint32_t now) {
    bool changed = false;
    if (pet->reaction != PET_REACT_NONE && !reacting(pet, now)) {
        pet->reaction = PET_REACT_NONE;
        changed       = true;
    }

    pet_mood_t mood = pet_mood_for(wpm, idle_ms);
    if (mood != pet->mood) {
        pet->mood      = mood;
        pet->step      = 0;
        pet->last_step = now;
        return true;
    }
    const pet_mood_def_t *def = &moods[mood];
    if (now - pet->last_step < def->interval_ms) {
        return changed;
    }
    // Swearing and panicking stop it in its tracks; sneaking creeps along.
    uint8_t speed = (swearing(pet, now) || pet->panic) ? 0 : pet->sneak ? 1 : def->speed;
    pet->step      = (pet->step + 1) % PET_STEPS;
    pet->x         = (uint8_t)((pet->x + speed) % PET_SCREEN_WIDTH);
    pet->last_step = now;
    return true;
}

uint8_t pet_sprite(const pet_t *pet) {
    return moods[pet->mood].sprites[pet->step];
}

void pet_hold(pet_t *pet, bool sneak, bool panic) {
    pet->sneak = sneak;
    pet->panic = panic;
}

void pet_react(pet_t *pet, pet_reaction_t reaction, uint32_t now) {
    // A swear reaction plays out before anything else gets a look in.
    if (swearing(pet, now) && reaction < PET_FIRST_SWEAR_REACTION) {
        return;
    }
    pet->reaction    = reaction;
    pet->reaction_at = now;
}

bool pet_animating(const pet_t *pet, uint32_t now) {
    return pet->panic || (reacting(pet, now) && pet->reaction != PET_REACT_FAINT);
}

// -1 or 1, flipping every period.
static int8_t wobble(uint32_t now, uint16_t period) {
    return (now / period) % 2 ? 1 : -1;
}

pet_pose_t pet_pose(const pet_t *pet, uint32_t now) {
    pet_pose_t pose    = {.sprite = pet_sprite(pet), .fx = -1};
    uint32_t   elapsed = now - pet->reaction_at;

    if (swearing(pet, now)) {
        switch (pet->reaction) {
            case PET_REACT_GRAWLIX:
                pose.sprite = PET_SPRITE_FLUSTERED;
                pose.dx     = wobble(now, 60);
                pose.fx     = PETFX_SPRITE_GRAWLIX;
                pose.fx_x   = 27;
                break;
            case PET_REACT_SOAP:
                pose.sprite = PET_SPRITE_FLUSTERED;
                pose.dx     = wobble(now, 120);
                pose.fx     = (elapsed / 300) % 2 ? PETFX_SPRITE_SOAP_2 : PETFX_SPRITE_SOAP_1;
                pose.fx_x   = 20;
                break;
            default:
                pose.sprite = PET_SPRITE_FAINT;
                break;
        }
        return pose;
    }
    if (pet->panic) {
        pose.sprite = PET_SPRITE_PANIC;
        pose.dx     = wobble(now, 50);
        pose.fx     = PETFX_SPRITE_ALERT;
        pose.fx_x   = 26;
        return pose;
    }
    if (pet->sneak) {
        pose.sprite = pet->step % 2 ? PET_SPRITE_CROUCH_2 : PET_SPRITE_CROUCH_1;
    }
    if (reacting(pet, now) && pet->reaction == PET_REACT_HOP) {
        // Parabola peaking halfway through the hop.
        uint32_t t = elapsed * 100 / reaction_ms[PET_REACT_HOP];
        pose.dy    = -(int8_t)(4 * HOP_HEIGHT * t * (100 - t) / 10000);
    }
    if (reacting(pet, now) && pet->reaction == PET_REACT_SWEAT) {
        pose.fx   = PETFX_SPRITE_SWEAT;
        pose.fx_x = 0;
        pose.fx_y = 2 + elapsed / 250; // the drop slides down
    }
    return pose;
}
