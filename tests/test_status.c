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

static void test_count_line(void) {
    status_line_t line;
    status_count_line(&line, 12345678, 1234);
    CHECK_STR(line.text, "ALL 12345678 NOW 1234");
    CHECK(line.invert == 0);
    status_count_line(&line, 0, 0);
    CHECK_STR(line.text, "ALL 0        NOW 0   ");
}

static void test_count_line_shortens_large_counts(void) {
    status_line_t line;
    status_count_line(&line, 123456789, 12345);
    CHECK_STR(line.text, "ALL 123456k  NOW 12k ");
    status_count_line(&line, 4294967295u, 1234567);
    CHECK_STR(line.text, "ALL 4294967k NOW 1M  ");
    status_count_line(&line, 999999999, 9999999);
    CHECK_STR(line.text, "ALL 999999k  NOW 9M  ");
}

int main(void) {
    test_layer_names();
    test_layer_line_without_mods();
    test_layer_line_inverts_held_mods();
    test_longest_layer_name_stays_in_tab();
    test_lock_line();
    test_lock_line_all_off();
    test_count_line();
    test_count_line_shortens_large_counts();
    return CHECK_DONE();
}
