#pragma once

#include QMK_KEYBOARD_H

// The latest pet reaction, synced to the non-USB half. seq and sweat_seq
// change with each new reaction or sweat so the pet knows to play it.
typedef struct {
    uint8_t seq;
    uint8_t reaction; // pet_reaction_t
    uint8_t sweat_seq;
} reactions_view_t;

const reactions_view_t *reactions_view(void);

void reactions_init(void);
// Watches keys that reach the host for the pet's reactions.
void reactions_record(uint16_t keycode, keyrecord_t *record);
void reactions_task(void);
