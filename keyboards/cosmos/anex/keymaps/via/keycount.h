#pragma once

// Lifetime and per-boot keypress counts, and when the lifetime count should
// be written back to storage. Free of QMK headers so it can be unit tested on
// the host.

#include <stdbool.h>
#include <stdint.h>

// Saves wait for a pause in typing, but long bursts still save periodically.
#define KEYCOUNT_SAVE_IDLE_MS 10000
#define KEYCOUNT_SAVE_EVERY 1000

typedef struct {
    uint32_t total;
    uint32_t session;
    uint32_t saved; // total as last written to storage
    uint32_t last_press;
} keycount_t;

void keycount_init(keycount_t *count, uint32_t stored);
void keycount_press(keycount_t *count, uint32_t now);
bool keycount_should_save(const keycount_t *count, uint32_t now);
void keycount_mark_saved(keycount_t *count);
