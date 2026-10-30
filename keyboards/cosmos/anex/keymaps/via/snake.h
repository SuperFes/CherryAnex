#pragma once

// Snake on a grid of 4x4 pixel cells covering a 128x64 display.
// Free of QMK headers so it can be unit tested on the host.

#include <stdbool.h>
#include <stdint.h>

#define SNAKE_CELL_PX 4
#define SNAKE_COLS 32
#define SNAKE_ROWS 16
#define SNAKE_CELLS (SNAKE_COLS * SNAKE_ROWS)
#define SNAKE_START_LENGTH 4
#define SNAKE_START_STEP_MS 160
#define SNAKE_MIN_STEP_MS 60
#define SNAKE_SPEEDUP_MS 4 // per food eaten
// Keeps a key mashed at the moment of death from starting the next round.
#define SNAKE_RESTART_DELAY_MS 600

typedef enum {
    SNAKE_UP,
    SNAKE_RIGHT,
    SNAKE_DOWN,
    SNAKE_LEFT,
} snake_dir_t;

typedef enum {
    SNAKE_READY, // waiting for the first turn
    SNAKE_PLAYING,
    SNAKE_OVER,
} snake_state_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} snake_cell_t;

typedef struct {
    snake_state_t state;
    snake_cell_t  body[SNAKE_CELLS]; // ring buffer, newest cell at body[head]
    uint16_t      head;
    uint16_t      length;
    uint8_t       occupied[SNAKE_CELLS / 8];
    snake_dir_t   dir;
    snake_dir_t   queued[2]; // turns pressed faster than the snake steps
    uint8_t       queued_count;
    snake_cell_t  food;
    uint16_t      score;
    uint32_t      last_step;
    uint32_t      over_at;
    uint32_t      rng;
} snake_t;

void snake_init(snake_t *snake, uint32_t seed);

// Starts the round from SNAKE_READY; reversing into the body is ignored.
void snake_turn(snake_t *snake, snake_dir_t dir, uint32_t now);

// Moves one cell once the step interval has passed. Returns true on change.
bool snake_update(snake_t *snake, uint32_t now);

uint16_t snake_step_ms(const snake_t *snake);
bool     snake_can_restart(const snake_t *snake, uint32_t now);
bool     snake_occupied(const snake_t *snake, uint8_t x, uint8_t y);

// Display byte for pixel column x of page (SSD1306 page format).
uint8_t snake_page_byte(const snake_t *snake, uint8_t x, uint8_t page);
