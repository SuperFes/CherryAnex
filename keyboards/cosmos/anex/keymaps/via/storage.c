#include "storage.h"

#define CHECK_MASK 0xA55A

void storage_pack(uint16_t value, uint8_t out[STORAGE_CHECKED_BYTES]) {
    uint16_t check = value ^ CHECK_MASK;
    out[0]         = value & 0xFF;
    out[1]         = value >> 8;
    out[2]         = check & 0xFF;
    out[3]         = check >> 8;
}

uint16_t storage_unpack(const uint8_t in[STORAGE_CHECKED_BYTES]) {
    uint16_t value = in[0] | (uint16_t)in[1] << 8;
    uint16_t check = in[2] | (uint16_t)in[3] << 8;
    return (value ^ CHECK_MASK) == check ? value : 0;
}
