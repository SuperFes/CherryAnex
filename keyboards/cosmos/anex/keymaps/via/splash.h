#pragma once

// Boot logo timing, measured from the first frame drawn rather than power-up,
// since USB detection eats into the start of the boot.
// Free of QMK headers so it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#define SPLASH_DURATION_MS 4000

typedef enum {
    SPLASH_SHOWING,
    SPLASH_FINISHED, // returned once, so the caller can clear the logo
    SPLASH_DONE,
} splash_state_t;

typedef struct {
    bool     started;
    bool     done;
    uint32_t start;
} splash_t;

splash_state_t splash_update(splash_t *splash, uint32_t now);
