#include "words.h"
#include "check.h"

// HID usage IDs.
enum { A = 0x04, B = 0x05, E = 0x08, H = 0x0B, L = 0x0F, O = 0x12, P = 0x13, S = 0x16, T = 0x17, ONE = 0x1E, ENTER = 0x28, BKSP = 0x2A, SPACE = 0x2C, COMMA = 0x36, LEFT = 0x50 };

static bool type(words_t *w, const uint8_t *keys, int n, char *finished) {
    bool done = false;
    for (int i = 0; i < n; ++i) {
        done = words_feed(w, keys[i], finished);
    }
    return done;
}

static void test_finishes_words_on_boundaries(void) {
    words_t w;
    char    finished[WORDS_MAX_LEN + 1];
    words_reset(&w);
    const uint8_t hello[] = {H, E, L, L, O, SPACE};
    CHECK(type(&w, hello, 6, finished));
    CHECK_STR(finished, "hello");

    const uint8_t comma[] = {T, O, P, COMMA};
    CHECK(type(&w, comma, 4, finished));
    CHECK_STR(finished, "top");

    const uint8_t enter[] = {A, ENTER};
    CHECK(type(&w, enter, 2, finished));
    CHECK_STR(finished, "a");
}

static void test_empty_and_reset_words_dont_finish(void) {
    words_t w;
    char    finished[WORDS_MAX_LEN + 1];
    words_reset(&w);
    CHECK(!words_feed(&w, SPACE, finished));
    // Moving the cursor leaves the word unknown.
    const uint8_t moved[] = {H, E, LEFT, SPACE};
    CHECK(!type(&w, moved, 4, finished));
    // Digits too.
    const uint8_t digit[] = {H, ONE, SPACE};
    CHECK(!type(&w, digit, 3, finished));
}

static void test_backspace_edits(void) {
    words_t w;
    char    finished[WORDS_MAX_LEN + 1];
    words_reset(&w);
    const uint8_t keys[] = {S, H, E, BKSP, O, P, BKSP, BKSP, BKSP, BKSP, T, O, P, SPACE};
    CHECK(type(&w, keys, sizeof(keys), finished));
    CHECK_STR(finished, "top");
}

static void test_overlong_words_never_finish(void) {
    words_t w;
    char    finished[WORDS_MAX_LEN + 1];
    words_reset(&w);
    for (int i = 0; i < WORDS_MAX_LEN + 1; ++i) {
        words_feed(&w, A, finished);
    }
    words_feed(&w, BKSP, finished);
    CHECK(!words_feed(&w, SPACE, finished));
}

// Known FNV-1a values, shared with tools/test_badwords.py.
static void test_hash_matches_reference(void) {
    CHECK(words_hash("", 0) == 0x811C9DC5u);
    CHECK(words_hash("a", 1) == 0xE40C292Cu);
    CHECK(words_hash("foobar", 6) == 0xBF9CF968u);
}

static void test_match_exact_and_prefix(void) {
    const uint32_t     exact[]  = {words_hash("heck", 4)};
    const uint32_t     prefix[] = {words_hash("frick", 5)};
    const words_list_t list     = {exact, 1, prefix, 1};
    CHECK(words_match(&list, "heck"));
    CHECK(!words_match(&list, "hecks"));
    CHECK(!words_match(&list, "check"));
    CHECK(words_match(&list, "frick"));
    CHECK(words_match(&list, "fricking"));
    CHECK(!words_match(&list, "fric"));
    CHECK(!words_match(&list, "hello"));
    const words_list_t empty = {0};
    CHECK(!words_match(&empty, "heck"));
}

int main(void) {
    test_finishes_words_on_boundaries();
    test_empty_and_reset_words_dont_finish();
    test_backspace_edits();
    test_overlong_words_never_finish();
    test_hash_matches_reference();
    test_match_exact_and_prefix();
    return CHECK_DONE();
}
