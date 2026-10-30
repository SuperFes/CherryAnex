#include "keycount.h"

void keycount_init(keycount_t *count, uint32_t stored) {
    *count = (keycount_t){.total = stored, .saved = stored};
}

void keycount_press(keycount_t *count, uint32_t now) {
    ++count->total;
    ++count->session;
    count->last_press = now;
}

bool keycount_should_save(const keycount_t *count, uint32_t now) {
    uint32_t unsaved = count->total - count->saved;
    return unsaved && (unsaved >= KEYCOUNT_SAVE_EVERY || now - count->last_press >= KEYCOUNT_SAVE_IDLE_MS);
}

void keycount_mark_saved(keycount_t *count) {
    count->saved = count->total;
}
