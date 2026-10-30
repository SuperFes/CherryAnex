#pragma once

// Screensaver scenes spanning both displays as one 256x64 field, left half
// first. Scenes are pure functions of a time shared by both halves, so they
// line up without syncing any state. Free of QMK headers so it can be unit
// tested on the host.

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"

#define SAVER_AFTER_MS 30000
// Overridden in config.h, which also ties the RGB timeout to it.
#ifndef SAVER_OFF_AFTER_MS
#    define SAVER_OFF_AFTER_MS 300000
#endif
#define SAVER_FIELD_WIDTH (2 * CANVAS_WIDTH)
#define SAVER_STAR_COUNT 48
#define SAVER_STAR_STEP_MS 50
#define SAVER_PET_SIZE 32
#define SAVER_PET_FRAME_MS 700

typedef enum {
    SAVER_STARS,
    SAVER_PET,
    SAVER_STYLES,
} saver_style_t;

saver_style_t saver_next_style(saver_style_t style);

typedef enum {
    SAVER_SCREEN_NORMAL,
    SAVER_SCREEN_SAVER,
    SAVER_SCREEN_OFF,
} saver_screen_t;

// What the displays show after idle_ms without input. preview shows the saver
// straight away, e.g. right after switching styles. A game keeps the saver away.
saver_screen_t saver_screen(uint32_t idle_ms, bool preview, bool game_active);

// Field position of star index at time t; stars drift right to left.
void saver_star(uint8_t index, uint32_t t, uint16_t *x, uint8_t *y);
void saver_draw_stars(canvas_t *canvas, uint32_t t, uint8_t half);

// Top-left field position of the sleeping pet at time t.
void saver_pet_position(uint32_t t, int16_t *x, int16_t *y);
// sleep_frames: two SAVER_PET_SIZE square sprites in page format.
void saver_draw_pet(canvas_t *canvas, uint32_t t, uint8_t half, const uint8_t *const sleep_frames[2]);
