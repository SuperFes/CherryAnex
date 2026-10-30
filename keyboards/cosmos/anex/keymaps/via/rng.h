#pragma once

#include <stdint.h>

// xorshift32; state must be non-zero.
static inline uint32_t rng_next(uint32_t *state) {
    uint32_t r = *state;
    r ^= r << 13;
    r ^= r >> 17;
    r ^= r << 5;
    return *state = r;
}
