#include "snake.h"

#include <string.h>

static const int8_t dx[] = {[SNAKE_UP] = 0, [SNAKE_RIGHT] = 1, [SNAKE_DOWN] = 0, [SNAKE_LEFT] = -1};
static const int8_t dy[] = {[SNAKE_UP] = -1, [SNAKE_RIGHT] = 0, [SNAKE_DOWN] = 1, [SNAKE_LEFT] = 0};

static uint16_t cell_index(uint8_t x, uint8_t y) {
    return (uint16_t)y * SNAKE_COLS + x;
}

static void set_occupied(snake_t *snake, snake_cell_t cell, bool on) {
    uint16_t i = cell_index(cell.x, cell.y);
    if (on) {
        snake->occupied[i / 8] |= 1u << (i % 8);
    } else {
        snake->occupied[i / 8] &= ~(1u << (i % 8));
    }
}

bool snake_occupied(const snake_t *snake, uint8_t x, uint8_t y) {
    uint16_t i = cell_index(x, y);
    return snake->occupied[i / 8] & (1u << (i % 8));
}

static uint32_t next_random(snake_t *snake) {
    // xorshift32
    uint32_t r = snake->rng;
    r ^= r << 13;
    r ^= r >> 17;
    r ^= r << 5;
    return snake->rng = r;
}

// Returns false when the board is full.
static bool place_food(snake_t *snake) {
    uint16_t free = SNAKE_CELLS - snake->length;
    if (!free) {
        return false;
    }
    uint16_t pick = next_random(snake) % free;
    for (uint8_t y = 0; y < SNAKE_ROWS; ++y) {
        for (uint8_t x = 0; x < SNAKE_COLS; ++x) {
            if (!snake_occupied(snake, x, y) && pick-- == 0) {
                snake->food = (snake_cell_t){x, y};
                return true;
            }
        }
    }
    return false;
}

void snake_init(snake_t *snake, uint32_t seed) {
    memset(snake, 0, sizeof(*snake));
    snake->rng    = seed ? seed : 1;
    snake->dir    = SNAKE_RIGHT;
    snake->length = SNAKE_START_LENGTH;
    snake->head   = SNAKE_START_LENGTH - 1;
    for (uint8_t i = 0; i < SNAKE_START_LENGTH; ++i) {
        snake->body[i] = (snake_cell_t){SNAKE_COLS / 2 - SNAKE_START_LENGTH + 1 + i, SNAKE_ROWS / 2};
        set_occupied(snake, snake->body[i], true);
    }
    place_food(snake);
}

void snake_turn(snake_t *snake, snake_dir_t dir, uint32_t now) {
    if (snake->state == SNAKE_OVER) {
        return;
    }
    if (snake->state == SNAKE_READY) {
        snake->state     = SNAKE_PLAYING;
        snake->last_step = now;
    }
    snake_dir_t last = snake->queued_count ? snake->queued[snake->queued_count - 1] : snake->dir;
    if (dir == last || dir == (last + 2) % 4 || snake->queued_count == sizeof(snake->queued) / sizeof(snake->queued[0])) {
        return;
    }
    snake->queued[snake->queued_count++] = dir;
}

uint16_t snake_step_ms(const snake_t *snake) {
    uint32_t faster = (uint32_t)snake->score * SNAKE_SPEEDUP_MS;
    return faster >= SNAKE_START_STEP_MS - SNAKE_MIN_STEP_MS ? SNAKE_MIN_STEP_MS : SNAKE_START_STEP_MS - faster;
}

bool snake_can_restart(const snake_t *snake, uint32_t now) {
    return snake->state == SNAKE_OVER && now - snake->over_at >= SNAKE_RESTART_DELAY_MS;
}

static void game_over(snake_t *snake, uint32_t now) {
    snake->state   = SNAKE_OVER;
    snake->over_at = now;
}

bool snake_update(snake_t *snake, uint32_t now) {
    if (snake->state != SNAKE_PLAYING || now - snake->last_step < snake_step_ms(snake)) {
        return false;
    }
    snake->last_step = now;

    if (snake->queued_count) {
        snake->dir       = snake->queued[0];
        snake->queued[0] = snake->queued[1];
        --snake->queued_count;
    }

    snake_cell_t from = snake->body[snake->head];
    int16_t      x    = from.x + dx[snake->dir];
    int16_t      y    = from.y + dy[snake->dir];
    if (x < 0 || x >= SNAKE_COLS || y < 0 || y >= SNAKE_ROWS) {
        game_over(snake, now);
        return true;
    }
    snake_cell_t next   = {(uint8_t)x, (uint8_t)y};
    bool         eating = next.x == snake->food.x && next.y == snake->food.y;

    // The tail moves out of the way first, so chasing it is legal.
    snake_cell_t tail = snake->body[(snake->head + SNAKE_CELLS - snake->length + 1) % SNAKE_CELLS];
    if (!eating) {
        set_occupied(snake, tail, false);
    }
    if (snake_occupied(snake, next.x, next.y)) {
        set_occupied(snake, tail, true);
        game_over(snake, now);
        return true;
    }

    snake->head              = (snake->head + 1) % SNAKE_CELLS;
    snake->body[snake->head] = next;
    set_occupied(snake, next, true);

    if (eating) {
        ++snake->length;
        ++snake->score;
        if (!place_food(snake)) {
            game_over(snake, now);
        }
    }
    return true;
}

bool snake_dir_for_usage(uint8_t usage, snake_dir_t *dir) {
    switch (usage) {
        case 0x52: // Up
        case 0x1A: // W
            *dir = SNAKE_UP;
            return true;
        case 0x4F: // Right
        case 0x07: // D
            *dir = SNAKE_RIGHT;
            return true;
        case 0x51: // Down
        case 0x16: // S
            *dir = SNAKE_DOWN;
            return true;
        case 0x50: // Left
        case 0x04: // A
            *dir = SNAKE_LEFT;
            return true;
        default:
            return false;
    }
}

#define BEST_CHECK 0xA55A

void snake_best_pack(uint16_t best, uint8_t out[SNAKE_BEST_BYTES]) {
    uint16_t check = best ^ BEST_CHECK;
    out[0]         = best & 0xFF;
    out[1]         = best >> 8;
    out[2]         = check & 0xFF;
    out[3]         = check >> 8;
}

uint16_t snake_best_unpack(const uint8_t in[SNAKE_BEST_BYTES]) {
    uint16_t best  = in[0] | (uint16_t)in[1] << 8;
    uint16_t check = in[2] | (uint16_t)in[3] << 8;
    return (best ^ BEST_CHECK) == check ? best : 0;
}

uint8_t snake_page_byte(const snake_t *snake, uint8_t x, uint8_t page) {
    uint8_t col = x / SNAKE_CELL_PX;
    uint8_t sub = x % SNAKE_CELL_PX;
    uint8_t out = 0;

    // Each page holds two cell rows: the upper one in the low nibble.
    for (uint8_t half = 0; half < 2; ++half) {
        uint8_t row = page * 2 + half;
        uint8_t bits;
        if (snake_occupied(snake, col, row)) {
            // 3x3 block, leaving a gap between body segments.
            bits = sub < 3 ? 0x07 : 0;
        } else if (snake->food.x == col && snake->food.y == row) {
            // Hollow diamond.
            bits = sub == 1 ? 0x05 : (sub == 0 || sub == 2) ? 0x02 : 0;
        } else {
            bits = 0;
        }
        out |= bits << (half * 4);
    }
    return out;
}
