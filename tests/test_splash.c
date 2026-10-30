#include "splash.h"
#include "check.h"

static void test_times_from_first_frame(void) {
    splash_t splash = {0};
    CHECK(splash_update(&splash, 3000) == SPLASH_SHOWING);
    CHECK(splash_update(&splash, 3000 + SPLASH_DURATION_MS - 1) == SPLASH_SHOWING);
    CHECK(splash_update(&splash, 3000 + SPLASH_DURATION_MS) == SPLASH_FINISHED);
}

static void test_finishes_once(void) {
    splash_t splash = {0};
    splash_update(&splash, 0);
    CHECK(splash_update(&splash, SPLASH_DURATION_MS) == SPLASH_FINISHED);
    CHECK(splash_update(&splash, SPLASH_DURATION_MS + 1) == SPLASH_DONE);
    CHECK(splash_update(&splash, 0) == SPLASH_DONE);
}

static void test_timer_wraparound(void) {
    splash_t splash = {0};
    splash_update(&splash, 0xFFFFFF00u);
    CHECK(splash_update(&splash, 0x100) == SPLASH_SHOWING);
    CHECK(splash_update(&splash, SPLASH_DURATION_MS) == SPLASH_FINISHED);
}

int main(void) {
    test_times_from_first_frame();
    test_finishes_once();
    test_timer_wraparound();
    return CHECK_DONE();
}
