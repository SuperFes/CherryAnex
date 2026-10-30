#pragma once

#include QMK_KEYBOARD_H
#include "keycount.h"

const keycount_t *stats_keycount(void);

void stats_init(void);
void stats_record(keyrecord_t *record);
void stats_task(void);
void stats_shutdown(void);
