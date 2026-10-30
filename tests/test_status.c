#include "status.h"
#include "check.h"

static uint32_t span(unsigned col, unsigned count) {
    return ((1UL << count) - 1) << col;
}

static void test_layer_names(void) {
    CHECK_STR(status_layer_name(0), "MAIN");
    CHECK_STR(status_layer_name(3), "RGB");
    CHECK_STR(status_layer_name(4), "INSANE!");
}

static void test_layer_line_without_mods(void) {
    status_line_t line;
    status_layer_line(&line, 1, (status_mods_t){0});
    CHECK_STR(line.text, " SYMBOL       C S A G");
    CHECK(line.invert == span(0, 8));
}

static void test_layer_line_inverts_held_mods(void) {
    status_line_t line;
    status_layer_line(&line, 0, (status_mods_t){.shift = true, .gui = true});
    CHECK_STR(line.text, " MAIN         C S A G");
    CHECK(line.invert == (span(0, 8) | span(16, 1) | span(20, 1)));
}

static void test_longest_layer_name_stays_in_tab(void) {
    status_line_t line;
    status_layer_line(&line, 9, (status_mods_t){0});
    CHECK_STR(line.text, " INSANE!      C S A G");
}

static void test_lock_line(void) {
    status_line_t line;
    status_lock_line(&line, (status_locks_t){.caps = true, .scroll = true});
    CHECK_STR(line.text, "CAPS    NUM      SCRL");
    CHECK(line.invert == (span(0, 4) | span(17, 4)));
}

static void test_lock_line_all_off(void) {
    status_line_t line;
    status_lock_line(&line, (status_locks_t){0});
    CHECK(line.invert == 0);
}

int main(void) {
    test_layer_names();
    test_layer_line_without_mods();
    test_layer_line_inverts_held_mods();
    test_longest_layer_name_stays_in_tab();
    test_lock_line();
    test_lock_line_all_off();
    return CHECK_DONE();
}
