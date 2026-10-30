#include "saver.h"

// Periods of the pet's bounce across the field, chosen not to line up.
#define PET_X_PERIOD_MS 40000
#define PET_Y_PERIOD_MS 13000
#define Z_RISE_MS 2400

// Page-format "z" glyphs, 5x5 and 4x4.
static const uint8_t z_big[]   = {0x11, 0x19, 0x15, 0x13, 0x11};
static const uint8_t z_small[] = {0x09, 0x0D, 0x0B, 0x09};

saver_style_t saver_next_style(saver_style_t style) {
    return (style + 1) % SAVER_STYLES;
}

saver_screen_t saver_screen(uint32_t idle_ms, bool preview, bool game_active) {
    if (idle_ms >= SAVER_OFF_AFTER_MS) {
        return SAVER_SCREEN_OFF;
    }
    if (!game_active && (preview || idle_ms >= SAVER_AFTER_MS)) {
        return SAVER_SCREEN_SAVER;
    }
    return SAVER_SCREEN_NORMAL;
}

static uint32_t mix(uint32_t h) {
    h *= 0x9E3779B1u;
    h ^= h >> 15;
    h *= 0x85EBCA77u;
    h ^= h >> 13;
    return h;
}

void saver_star(uint8_t index, uint32_t t, uint16_t *x, uint8_t *y) {
    uint32_t h     = mix(index + 1);
    uint8_t  speed = 1 + (h >> 6) % 3;
    uint32_t start = (h >> 12) % SAVER_FIELD_WIDTH;
    uint32_t moved = (t / SAVER_STAR_STEP_MS) * speed % SAVER_FIELD_WIDTH;

    *x = (start + SAVER_FIELD_WIDTH - moved) % SAVER_FIELD_WIDTH;
    *y = h % CANVAS_HEIGHT;
}

void saver_draw_stars(canvas_t *canvas, uint32_t t, uint8_t half) {
    canvas_clear(canvas);
    for (uint8_t i = 0; i < SAVER_STAR_COUNT; ++i) {
        uint16_t x;
        uint8_t  y;
        saver_star(i, t, &x, &y);
        int16_t local = (int16_t)x - half * CANVAS_WIDTH;
        canvas_pixel(canvas, local, y);
        // The fastest stars get a trail.
        if (1 + (mix(i + 1) >> 6) % 3 == 3) {
            canvas_pixel(canvas, local + 1, y);
        }
    }
}

// 0 -> span -> 0 over period.
static int16_t bounce(uint32_t t, uint32_t period, int16_t span) {
    uint32_t phase = t % period;
    uint32_t half  = period / 2;
    uint32_t pos   = phase < half ? phase : period - phase;
    return (int16_t)(pos * span / half);
}

void saver_pet_position(uint32_t t, int16_t *x, int16_t *y) {
    *x = bounce(t, PET_X_PERIOD_MS, SAVER_FIELD_WIDTH - SAVER_PET_SIZE);
    *y = bounce(t, PET_Y_PERIOD_MS, CANVAS_HEIGHT - SAVER_PET_SIZE);
}

void saver_draw_pet(canvas_t *canvas, uint32_t t, uint8_t half, const uint8_t *const sleep_frames[2]) {
    int16_t x, y;
    saver_pet_position(t, &x, &y);
    x -= half * CANVAS_WIDTH;

    canvas_clear(canvas);
    canvas_blit(canvas, sleep_frames[(t / SAVER_PET_FRAME_MS) % 2], SAVER_PET_SIZE, SAVER_PET_SIZE, x, y);

    // Two z's float up from the pet's head, half a cycle apart.
    for (uint8_t i = 0; i < 2; ++i) {
        uint32_t rise = (t + i * Z_RISE_MS / 2) % Z_RISE_MS;
        int16_t  zx   = x + SAVER_PET_SIZE - 6 + (int16_t)(rise * 8 / Z_RISE_MS);
        int16_t  zy   = y + 2 - (int16_t)(rise * 16 / Z_RISE_MS);
        if (rise < Z_RISE_MS / 2) {
            canvas_blit(canvas, z_small, sizeof(z_small), 8, zx, zy);
        } else {
            canvas_blit(canvas, z_big, sizeof(z_big), 8, zx, zy);
        }
    }
}
