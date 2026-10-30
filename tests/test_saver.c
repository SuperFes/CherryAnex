#include "saver.h"
#include "check.h"

static canvas_t canvas;

static int lit(int16_t x, int16_t y) {
    return (canvas.bytes[(y / 8) * CANVAS_WIDTH + x] >> (y % 8)) & 1;
}

static int lit_count(void) {
    int n = 0;
    for (int i = 0; i < CANVAS_BYTES; ++i) {
        for (uint8_t b = canvas.bytes[i]; b; b &= b - 1) {
            ++n;
        }
    }
    return n;
}

static void test_showing_rules(void) {
    CHECK(!saver_showing(SAVER_AFTER_MS - 1, false, false));
    CHECK(saver_showing(SAVER_AFTER_MS, false, false));
    CHECK(!saver_showing(SAVER_AFTER_MS, false, true));
    CHECK(saver_showing(0, true, false));
}

static void test_styles_cycle(void) {
    CHECK(saver_next_style(SAVER_STARS) == SAVER_PET);
    CHECK(saver_next_style(SAVER_PET) == SAVER_STARS);
}

static void test_stars_drift_left_and_wrap(void) {
    for (uint8_t i = 0; i < SAVER_STAR_COUNT; ++i) {
        uint16_t x0, x1;
        uint8_t  y0, y1;
        saver_star(i, 0, &x0, &y0);
        saver_star(i, SAVER_STAR_STEP_MS, &x1, &y1);
        uint16_t moved = (x0 + SAVER_FIELD_WIDTH - x1) % SAVER_FIELD_WIDTH;
        CHECK(moved >= 1 && moved <= 3);
        CHECK(y0 == y1 && y0 < CANVAS_HEIGHT);
        CHECK(x0 < SAVER_FIELD_WIDTH && x1 < SAVER_FIELD_WIDTH);
    }
}

static void test_stars_split_across_halves(void) {
    uint32_t t = 123456;
    int      total = 0;
    for (uint8_t half = 0; half < 2; ++half) {
        saver_draw_stars(&canvas, t, half);
        total += lit_count();
    }
    CHECK(total >= SAVER_STAR_COUNT / 2);

    // A star is drawn on whichever half its field position falls in.
    uint16_t x;
    uint8_t  y;
    saver_star(0, t, &x, &y);
    saver_draw_stars(&canvas, t, x / CANVAS_WIDTH);
    CHECK(lit(x % CANVAS_WIDTH, y));
}

static void test_pet_stays_in_field(void) {
    for (uint32_t t = 0; t < 100000; t += 97) {
        int16_t x, y;
        saver_pet_position(t, &x, &y);
        CHECK(x >= 0 && x <= SAVER_FIELD_WIDTH - SAVER_PET_SIZE);
        CHECK(y >= 0 && y <= CANVAS_HEIGHT - SAVER_PET_SIZE);
    }
}

static void test_pet_drawn_on_its_half(void) {
    static uint8_t        solid[SAVER_PET_SIZE * SAVER_PET_SIZE / 8];
    const uint8_t *const  frames[2] = {solid, solid};
    for (unsigned i = 0; i < sizeof(solid); ++i) {
        solid[i] = 0xFF;
    }
    // Start of the bounce: pet at the left edge of the left half.
    saver_draw_pet(&canvas, 0, 0, frames);
    CHECK(lit(0, 0) && lit(SAVER_PET_SIZE - 1, SAVER_PET_SIZE - 1));
    saver_draw_pet(&canvas, 0, 1, frames);
    CHECK(lit_count() == 0);
}

int main(void) {
    test_showing_rules();
    test_styles_cycle();
    test_stars_drift_left_and_wrap();
    test_stars_split_across_halves();
    test_pet_stays_in_field();
    test_pet_drawn_on_its_half();
    return CHECK_DONE();
}
