#pragma once

// Layout of the EECONFIG user datablock (EECONFIG_USER_DATA_SIZE in config.h),
// plus packing for fields added after the first release. Free of QMK headers
// so it can be unit tested on the host.

#include <stdint.h>

#define STORAGE_KEYCOUNT_OFFSET 0
#define STORAGE_SNAKE_BEST_OFFSET 4
#define STORAGE_SAVER_STYLE_OFFSET 8
#define STORAGE_RUNNER_BEST_OFFSET 12
#define STORAGE_SIZE 16

// A 16-bit value plus a check word, so storage that was never written (or
// held something else before the block grew) reads back as 0.
#define STORAGE_CHECKED_BYTES 4
void     storage_pack(uint16_t value, uint8_t out[STORAGE_CHECKED_BYTES]);
uint16_t storage_unpack(const uint8_t in[STORAGE_CHECKED_BYTES]);
