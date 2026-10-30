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

typedef struct {
    pet_mood_t mood;
    uint8_t    step;
    uint8_t    x; // left edge of the sprite; drawing wraps at PET_SCREEN_WIDTH
    uint32_t   last_step;
} pet_t;

pet_mood_t pet_mood_for(uint8_t wpm, uint32_t idle_ms);

// Advances the animation if its step interval has passed. Returns true when
// the sprite or its position changed and the pet needs redrawing.
bool pet_update(pet_t *pet, uint8_t wpm, uint32_t idle_ms, uint32_t now);

// Index into the sprite table generated from art/pet.txt.
uint8_t pet_sprite(const pet_t *pet);
