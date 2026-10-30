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
    status_lock_line(&line, (status_locks_t){.caps = true, .scroll = true}, NULL);
    CHECK_STR(line.text, "CAPS NUM SCRL        ");
    CHECK(line.invert == (span(0, 4) | span(9, 4)));
}

static void test_lock_line_with_clock(void) {
    status_line_t line;
    status_lock_line(&line, (status_locks_t){.num = true}, &(status_pc_t){.hour = 9, .minute = 5});
    CHECK_STR(line.text, "CAPS NUM SCRL   09:05");
    CHECK(line.invert == span(5, 3));
}

static void test_lock_line_all_off(void) {
    status_line_t line;
    status_lock_line(&line, (status_locks_t){0}, NULL);
    CHECK(line.invert == 0);
}

static void test_wpm_line_with_peak(void) {
    status_line_t line;
    status_wpm_line(&line, 5, 123, NULL);
    CHECK_STR(line.text, "WPM   5     PEAK 123 ");
    status_wpm_line(&line, 0, 0, NULL);
    CHECK_STR(line.text, "WPM   0     PEAK   0 ");
}

static void test_wpm_line_with_pc(void) {
    status_line_t line;
    status_wpm_line(&line, 87, 200, &(status_pc_t){.cpu = 7, .ram = 64});
    CHECK_STR(line.text, "WPM  87 CPU  7 RAM 64");
    // 100% doesn't fit in two columns.
    status_wpm_line(&line, 240, 0, &(status_pc_t){.cpu = 100, .ram = 100});
    CHECK_STR(line.text, "WPM 240 CPU 99 RAM 99");
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

static void test_score_line(void) {
    status_line_t line;
    status_score_line(&line, 12, 345);
    CHECK_STR(line.text, "SCORE 12    BEST 345 ");
    CHECK(line.invert == 0);
}

int main(void) {
    test_layer_names();
    test_layer_line_without_mods();
    test_layer_line_inverts_held_mods();
    test_longest_layer_name_stays_in_tab();
    test_lock_line();
    test_lock_line_all_off();
    test_lock_line_with_clock();
    test_wpm_line_with_peak();
    test_wpm_line_with_pc();
    test_count_line();
    test_count_line_shortens_large_counts();
    test_score_line();
    return CHECK_DONE();
}
