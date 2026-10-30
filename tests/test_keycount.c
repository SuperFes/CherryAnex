#include "keycount.h"
#include "check.h"

static void test_init_resumes_stored_total(void) {
    keycount_t count;
    keycount_init(&count, 500);
    CHECK(count.total == 500);
    CHECK(count.session == 0);
    CHECK(!keycount_should_save(&count, KEYCOUNT_SAVE_IDLE_MS * 2));
}

static void test_press_counts_both(void) {
    keycount_t count;
    keycount_init(&count, 500);
    keycount_press(&count, 10);
    keycount_press(&count, 20);
    CHECK(count.total == 502);
    CHECK(count.session == 2);
}

static void test_saves_after_idle(void) {
    keycount_t count;
    keycount_init(&count, 0);
    keycount_press(&count, 1000);
    CHECK(!keycount_should_save(&count, 1000 + KEYCOUNT_SAVE_IDLE_MS - 1));
    CHECK(keycount_should_save(&count, 1000 + KEYCOUNT_SAVE_IDLE_MS));
    keycount_mark_saved(&count);
    CHECK(!keycount_should_save(&count, 1000 + KEYCOUNT_SAVE_IDLE_MS));
}

static void test_saves_during_long_bursts(void) {
    keycount_t count;
    keycount_init(&count, 0);
    for (uint32_t i = 0; i < KEYCOUNT_SAVE_EVERY - 1; ++i) {
        keycount_press(&count, i);
    }
    CHECK(!keycount_should_save(&count, KEYCOUNT_SAVE_EVERY));
    keycount_press(&count, KEYCOUNT_SAVE_EVERY);
    CHECK(keycount_should_save(&count, KEYCOUNT_SAVE_EVERY));
}

static void test_idle_timer_wraparound(void) {
    keycount_t count;
    keycount_init(&count, 0);
    keycount_press(&count, 0xFFFFFF00u);
    CHECK(!keycount_should_save(&count, 0x100));
    CHECK(keycount_should_save(&count, KEYCOUNT_SAVE_IDLE_MS));
}

int main(void) {
    test_init_resumes_stored_total();
    test_press_counts_both();
    test_saves_after_idle();
    test_saves_during_long_bursts();
    test_idle_timer_wraparound();
    return CHECK_DONE();
}
