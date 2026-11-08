#pragma once

// Stands in for QMK's progmem.h so glcdfont.c and the sprite headers build on the PC.

#define PROGMEM
#define memcpy_P memcpy
#define pgm_read_byte(p) (*(const uint8_t *)(p))
