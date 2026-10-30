#include "storage.h"
#include "check.h"

static void test_round_trips(void) {
    uint8_t bytes[STORAGE_CHECKED_BYTES];
    storage_pack(321, bytes);
    CHECK(storage_unpack(bytes) == 321);
    storage_pack(0xFFFF, bytes);
    CHECK(storage_unpack(bytes) == 0xFFFF);
    storage_pack(0, bytes);
    CHECK(storage_unpack(bytes) == 0);
}

// Same encoding as the snake best score shipped before these helpers existed.
static void test_layout_is_stable(void) {
    uint8_t bytes[STORAGE_CHECKED_BYTES];
    storage_pack(0x0102, bytes);
    CHECK(bytes[0] == 0x02 && bytes[1] == 0x01 && bytes[2] == 0x58 && bytes[3] == 0xA4);
}

static void test_ignores_unwritten_storage(void) {
    const uint8_t erased[STORAGE_CHECKED_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t zeroed[STORAGE_CHECKED_BYTES] = {0x00, 0x00, 0x00, 0x00};
    const uint8_t stale[STORAGE_CHECKED_BYTES]  = {0x12, 0x34, 0x56, 0x78};
    CHECK(storage_unpack(erased) == 0);
    CHECK(storage_unpack(zeroed) == 0);
    CHECK(storage_unpack(stale) == 0);
}

int main(void) {
    test_round_trips();
    test_layout_is_stable();
    test_ignores_unwritten_storage();
    return CHECK_DONE();
}
