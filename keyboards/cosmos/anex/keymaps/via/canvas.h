#pragma once

// Off-screen 128x64 buffer in SSD1306 page format, for scenes that place
// sprites at arbitrary pixel positions. Free of QMK headers so it can be unit
// tested on the host.

#include <stdint.h>

#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BYTES (CANVAS_WIDTH * CANVAS_HEIGHT / 8)

typedef struct {
    uint8_t bytes[CANVAS_BYTES];
} canvas_t;

void canvas_clear(canvas_t *canvas);

// Drawing outside the canvas is clipped.
void canvas_pixel(canvas_t *canvas, int16_t x, int16_t y);

// ORs in a page-format sprite (height a multiple of 8) with its top-left
// corner at (x, y).
void canvas_blit(canvas_t *canvas, const uint8_t *sprite, uint8_t width, uint8_t height, int16_t x, int16_t y);
