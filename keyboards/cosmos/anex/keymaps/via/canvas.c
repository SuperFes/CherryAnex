#include "canvas.h"

#include <string.h>

#define PAGES (CANVAS_HEIGHT / 8)

void canvas_clear(canvas_t *canvas) {
    memset(canvas->bytes, 0, sizeof(canvas->bytes));
}

void canvas_pixel(canvas_t *canvas, int16_t x, int16_t y) {
    if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT) {
        return;
    }
    canvas->bytes[(y / 8) * CANVAS_WIDTH + x] |= 1u << (y % 8);
}

static void or_byte(canvas_t *canvas, int16_t page, int16_t x, uint8_t bits) {
    if (bits && page >= 0 && page < PAGES) {
        canvas->bytes[page * CANVAS_WIDTH + x] |= bits;
    }
}

void canvas_blit(canvas_t *canvas, const uint8_t *sprite, uint8_t width, uint8_t height, int16_t x, int16_t y) {
    // Floor division, so sprites partly above the top edge still line up.
    int16_t first_page = y >= 0 ? y / 8 : -((7 - y) / 8);
    uint8_t shift      = y - first_page * 8;

    for (uint8_t sprite_page = 0; sprite_page < height / 8; ++sprite_page) {
        int16_t page = first_page + sprite_page;
        for (uint8_t col = 0; col < width; ++col) {
            int16_t dest = x + col;
            if (dest < 0 || dest >= CANVAS_WIDTH) {
                continue;
            }
            uint8_t bits = sprite[sprite_page * width + col];
            or_byte(canvas, page, dest, bits << shift);
            if (shift) {
                or_byte(canvas, page + 1, dest, bits >> (8 - shift));
            }
        }
    }
}
