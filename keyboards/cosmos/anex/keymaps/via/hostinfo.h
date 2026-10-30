#pragma once

// PC stats sent by the anex-host app over VIA's raw HID channel.
// Free of QMK headers so it can be unit tested on the host.
//
// Report layout (32 bytes, unused bytes zero):
//   [0] HOST_INFO_COMMAND  [1] HOST_INFO_VERSION
//   [2] hour 0-23  [3] minute 0-59  [4] CPU % 0-100  [5] RAM % 0-100

#include <stdbool.h>
#include <stdint.h>

// Outside VIA's command IDs, so VIA never sees these reports.
#define HOST_INFO_COMMAND 0x80
#define HOST_INFO_VERSION 1
#define HOST_INFO_LENGTH 6
// The app sends every second; the stats are hidden once it stops.
#define HOST_INFO_TIMEOUT_MS 5000

typedef struct {
    bool     valid;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  cpu;
    uint8_t  ram;
    uint32_t received_at;
} host_info_t;

// Returns true when the report is a host info command, so VIA must not handle
// it. Only well-formed reports update info.
bool host_info_receive(host_info_t *info, const uint8_t *data, uint8_t length, uint32_t now);

bool host_info_fresh(const host_info_t *info, uint32_t now);
