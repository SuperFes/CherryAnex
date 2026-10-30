#pragma once

#include QMK_KEYBOARD_H
#include "saver.h"

// Custom keycode VIA shows as "Saver" (CUSTOM(1) in via-definition.json).
#define SAVER_TOGGLE QK_KB_1

// Synced to the non-USB half.
typedef struct {
    uint8_t style; // saver_style_t
    bool    preview;
} screensaver_view_t;

const screensaver_view_t *screensaver_view(void);

void screensaver_init(void);
// Returns true when the key was the saver toggle, so it must not reach the host.
bool screensaver_record(uint16_t keycode, keyrecord_t *record);
void screensaver_task(void);
