#include "words.h"

#include <string.h>

#define USAGE_A 0x04
#define USAGE_Z 0x1D
#define USAGE_ENTER 0x28
#define USAGE_BACKSPACE 0x2A
#define USAGE_TAB 0x2B
#define USAGE_SPACE 0x2C
#define USAGE_FIRST_PUNCT 0x2D // - = [ ] \ # ; ' ` , . /
#define USAGE_LAST_PUNCT 0x38

void words_reset(words_t *words) {
    memset(words, 0, sizeof(*words));
}

static bool ends_word(uint8_t usage) {
    return usage == USAGE_ENTER || usage == USAGE_TAB || usage == USAGE_SPACE || (usage >= USAGE_FIRST_PUNCT && usage <= USAGE_LAST_PUNCT);
}

bool words_feed(words_t *words, uint8_t usage, char finished[WORDS_MAX_LEN + 1]) {
    if (usage >= USAGE_A && usage <= USAGE_Z) {
        if (words->len < WORDS_MAX_LEN) {
            words->text[words->len++] = 'a' + (usage - USAGE_A);
            words->text[words->len]   = '\0';
        } else {
            words->overflow = true;
        }
        return false;
    }
    if (usage == USAGE_BACKSPACE) {
        if (words->overflow) {
            // The dropped letters are unknown, so the word can't be trusted.
            return false;
        }
        if (words->len) {
            words->text[--words->len] = '\0';
        }
        return false;
    }

    bool done = ends_word(usage) && words->len && !words->overflow;
    if (done) {
        memcpy(finished, words->text, words->len + 1);
    }
    // Anything else, like arrows or a click elsewhere, leaves the word unknown.
    words_reset(words);
    return done;
}

uint32_t words_hash(const char *text, uint8_t len) {
    uint32_t hash = 0x811C9DC5u;
    for (uint8_t i = 0; i < len; ++i) {
        hash ^= (uint8_t)text[i];
        hash *= 0x01000193u;
    }
    return hash;
}

static bool contains(const uint32_t *hashes, uint16_t count, uint32_t hash) {
    for (uint16_t i = 0; i < count; ++i) {
        if (hashes[i] == hash) {
            return true;
        }
    }
    return false;
}

bool words_match(const words_list_t *list, const char *word) {
    uint8_t len = strlen(word);
    if (contains(list->exact, list->exact_count, words_hash(word, len))) {
        return true;
    }
    for (uint8_t i = 1; i <= len; ++i) {
        if (contains(list->prefix, list->prefix_count, words_hash(word, i))) {
            return true;
        }
    }
    return false;
}
