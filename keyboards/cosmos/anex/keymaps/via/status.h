#pragma once

// Formats the status strip shared by both halves. Kept free of QMK headers so
// it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#define STATUS_COLS 21

typedef struct {
    char     text[STATUS_COLS + 1];
    uint32_t invert; // bit n set: column n is drawn inverted
} status_line_t;

typedef struct {
    bool ctrl;
    bool shift;
    bool alt;
    bool gui;
} status_mods_t;

typedef struct {
    bool caps;
    bool num;
    bool scroll;
} status_locks_t;

const char *status_layer_name(uint8_t layer);

// Layer name as an inverted tab on the left, mod letters on the right.
void status_layer_line(status_line_t *line, uint8_t layer, status_mods_t mods);

// Lock key labels, inverted while the lock is on.
void status_lock_line(status_line_t *line, status_locks_t locks);
