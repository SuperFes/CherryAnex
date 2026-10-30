#pragma once

// State the non-USB half's display needs from the master.
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_OLED_ENABLE
#define SPLIT_WPM_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_KEYCOUNT, USER_SYNC_GAME

// See storage.h. The version stays at the original block size so growing the
// block keeps the stored keypress total; later fields carry their own check.
#define EECONFIG_USER_DATA_SIZE 8
#define EECONFIG_USER_DATA_VERSION 4

// QMK's WPM smoothing overshoots when the main loop runs slower than its
// 100ms window (OLED flushes), wrapping the uint8 result to ~255.
#define WPM_UNFILTERED
