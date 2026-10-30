#pragma once

#include QMK_KEYBOARD_H

// PC stats as shown on either half; synced to the non-USB half.
typedef struct {
    bool    connected;
    uint8_t hour;
    uint8_t minute;
    uint8_t cpu;
    uint8_t ram;
} pc_view_t;

const pc_view_t *pc_view(void);

void pc_init(void);
// Returns true when the report was ours and VIA must not handle or answer it.
bool pc_receive(uint8_t *data, uint8_t length);
void pc_task(void);
