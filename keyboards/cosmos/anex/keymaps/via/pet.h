#pragma once

// Typing-driven pet for the right display: which sprite to show and where.
// Free of QMK headers so it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#define PET_STEPS 4
#define PET_SCREEN_WIDTH 128
#define PET_SLEEP_AFTER_MS 30000
#define PET_RUN_WPM 40

typedef enum {
    PET_SLEEP,
    PET_IDLE,
    PET_WALK,
    PET_RUN,
} pet_mood_t;

// One-off reactions to typing; the swear ones take precedence over the rest.
typedef enum {
    PET_REACT_NONE,
    PET_REACT_HOP,      // Space
    PET_REACT_BACKSTEP, // Backspace: a careful step back
    PET_REACT_GRAWLIX,
    PET_REACT_SOAP,
    PET_REACT_FAINT,
    PET_REACTIONS,
} pet_reaction_t;

#define PET_FIRST_SWEAR_REACTION PET_REACT_GRAWLIX
#define PET_SWEAR_REACTIONS (PET_REACTIONS - PET_FIRST_SWEAR_REACTION)

typedef struct {
    pet_mood_t     mood;
    uint8_t        step;
    uint8_t        x; // left edge of the sprite; drawing wraps at PET_SCREEN_WIDTH
    uint32_t       last_step;
    pet_reaction_t reaction;
    uint32_t       reaction_at;
    uint8_t        backsteps; // alternates feet on each step back
    bool           sweating;  // after a run of Backspace; overlays other reactions
    uint32_t       sweat_at;
    bool           sneak; // Ctrl held
    bool           panic; // Caps Lock on
} pet_t;

// Where and how to draw the pet, relative to its usual spot.
typedef struct {
    uint8_t sprite; // PET_SPRITE_*
    int8_t  dx;
    int8_t  dy;
    int8_t  fx; // PETFX_SPRITE_*, or -1 for none
    int8_t  fx_x;
    int8_t  fx_y;
} pet_pose_t;

pet_mood_t pet_mood_for(uint8_t wpm, uint32_t idle_ms);

// Advances the animation if its step interval has passed. Returns true when
// the sprite or its position changed and the pet needs redrawing.
bool pet_update(pet_t *pet, uint8_t wpm, uint32_t idle_ms, uint32_t now);

// Index into the sprite table generated from art/pet.txt, for the mood alone.
uint8_t pet_sprite(const pet_t *pet);

// States held by keys; call before pet_update.
void pet_hold(pet_t *pet, bool sneak, bool panic);
void pet_react(pet_t *pet, pet_reaction_t reaction, uint32_t now);
void pet_sweat(pet_t *pet, uint32_t now);

// True while the pose changes every frame, so it needs constant redrawing.
bool       pet_animating(const pet_t *pet, uint32_t now);
pet_pose_t pet_pose(const pet_t *pet, uint32_t now);
