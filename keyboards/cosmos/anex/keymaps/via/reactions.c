#include "reactions.h"

#include <string.h>

#include "badword_hashes.h"
#include "keys.h"
#include "pet.h"
#include "rng.h"
#include "transactions.h"
#include "words.h"

#define SYNC_INTERVAL_MS 30
// Backspaces in a row before the pet starts sweating.
#define SWEAT_AFTER 5

static const words_list_t badwords = {badword_exact, BADWORD_EXACT_COUNT, badword_prefix, BADWORD_PREFIX_COUNT};

static reactions_view_t view;
static words_t          word;
static uint8_t          backspaces;
static uint32_t         rng;

const reactions_view_t *reactions_view(void) {
    return &view;
}

static void receive_sync(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(view)) {
        memcpy(&view, in_data, sizeof(view));
    }
}

static void send_sync(void) {
    static uint32_t         last_attempt;
    static reactions_view_t sent;
    static bool             synced;

    if ((synced && memcmp(&sent, &view, sizeof(view)) == 0) || timer_elapsed32(last_attempt) < SYNC_INTERVAL_MS) {
        return;
    }
    last_attempt = timer_read32();
    if (transaction_rpc_send(USER_SYNC_REACTION, sizeof(view), &view)) {
        sent   = view;
        synced = true;
    }
}

static void react(pet_reaction_t reaction) {
    view.reaction = reaction;
    ++view.seq;
}

void reactions_init(void) {
    words_reset(&word);
    transaction_register_rpc(USER_SYNC_REACTION, receive_sync);
}

void reactions_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return;
    }
    uint8_t usage = key_usage(keycode);

    // Shortcuts aren't typing; Ctrl+Backspace and friends also lose track of the word.
    if (get_mods() & (MOD_MASK_CTRL | MOD_MASK_ALT | MOD_MASK_GUI)) {
        words_reset(&word);
        backspaces = 0;
        return;
    }

    backspaces = usage == KC_BACKSPACE ? backspaces + 1 : 0;
    if (backspaces == SWEAT_AFTER) {
        ++view.sweat_seq;
    }

    char finished[WORDS_MAX_LEN + 1];
    if (words_feed(&word, usage, finished) && words_match(&badwords, finished)) {
        if (!rng) {
            rng = timer_read32() | 1;
        }
        react(PET_FIRST_SWEAR_REACTION + rng_next(&rng) % PET_SWEAR_REACTIONS);
    } else if (usage == KC_SPACE) {
        react(PET_REACT_HOP);
    } else if (usage == KC_BACKSPACE) {
        react(PET_REACT_BACKSTEP);
    }
}

void reactions_task(void) {
    if (is_keyboard_master()) {
        send_sync();
    }
}
