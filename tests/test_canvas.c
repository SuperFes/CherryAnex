#include "canvas.h"
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

// 2x8 sprite: top pixel of column 0, bottom pixel of column 1.
static const uint8_t corners[] = {0x01, 0x80};

static void test_pixel_and_clipping(void) {
    canvas_clear(&canvas);
    canvas_pixel(&canvas, 5, 9);
    CHECK(lit(5, 9));
    canvas_pixel(&canvas, -1, 0);
    canvas_pixel(&canvas, CANVAS_WIDTH, 0);
    canvas_pixel(&canvas, 0, -1);
    canvas_pixel(&canvas, 0, CANVAS_HEIGHT);
    CHECK(lit_count() == 1);
}

static void test_blit_aligned(void) {
    canvas_clear(&canvas);
    canvas_blit(&canvas, corners, 2, 8, 10, 16);
    CHECK(lit(10, 16) && lit(11, 23));
    CHECK(lit_count() == 2);
}

static void test_blit_unaligned_spans_pages(void) {
    canvas_clear(&canvas);
    canvas_blit(&canvas, corners, 2, 8, 10, 13);
    CHECK(lit(10, 13) && lit(11, 20));
    CHECK(lit_count() == 2);
}

static void test_blit_clips_every_edge(void) {
    canvas_clear(&canvas);
    canvas_blit(&canvas, corners, 2, 8, -1, -7); // only column 1's bottom pixel, at (0, 0)
    CHECK(lit(0, 0));
    CHECK(lit_count() == 1);

    canvas_clear(&canvas);
    canvas_blit(&canvas, corners, 2, 8, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1); // only column 0's top pixel
    CHECK(lit(CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1));
    CHECK(lit_count() == 1);

    canvas_clear(&canvas);
    canvas_blit(&canvas, corners, 2, 8, -100, 100);
    CHECK(lit_count() == 0);
}

static void test_blit_ors_with_existing(void) {
    canvas_clear(&canvas);
    canvas_pixel(&canvas, 10, 17);
    canvas_blit(&canvas, corners, 2, 8, 10, 16);
    CHECK(lit(10, 16) && lit(10, 17) && lit(11, 23));
}

int main(void) {
    test_pixel_and_clipping();
    test_blit_aligned();
    test_blit_unaligned_spans_pages();
    test_blit_clips_every_edge();
    test_blit_ors_with_existing();
    return CHECK_DONE();
}
