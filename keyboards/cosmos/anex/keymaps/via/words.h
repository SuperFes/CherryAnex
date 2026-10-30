#pragma once

// Tracks the word being typed and checks finished words against hashed word
// lists. Only the current word is kept, in RAM. Keys are HID usage IDs, so
// this assumes a QWERTY layout on the host.
// Free of QMK headers so it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#define WORDS_MAX_LEN 16

typedef struct {
    char    text[WORDS_MAX_LEN + 1];
    uint8_t len;
    bool    overflow; // longer than WORDS_MAX_LEN; never matches
} words_t;

typedef struct {
    const uint32_t *exact;
    uint16_t        exact_count;
    // Entries match any word starting with them.
    const uint32_t *prefix;
    uint16_t        prefix_count;
} words_list_t;

void words_reset(words_t *words);

// Feeds one key press. Returns true when it ended a word, copying it
// (lowercase) into finished.
bool words_feed(words_t *words, uint8_t usage, char finished[WORDS_MAX_LEN + 1]);

// 32-bit FNV-1a over the first len characters; tools/badwords.py must agree.
uint32_t words_hash(const char *text, uint8_t len);

bool words_match(const words_list_t *list, const char *word);
