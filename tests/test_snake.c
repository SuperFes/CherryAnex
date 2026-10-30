#include "snake.h"
#include "check.h"

static snake_cell_t head(const snake_t *s) {
    return s->body[s->head];
}

// Food parked in a corner the tests never reach.
static void start(snake_t *s) {
    snake_init(s, 1234);
    s->food = (snake_cell_t){0, 0};
}

static uint32_t step(snake_t *s, uint32_t now) {
    now += snake_step_ms(s);
    CHECK(snake_update(s, now));
    return now;
}

static void test_init(void) {
    snake_t s;
    snake_init(&s, 1234);
    CHECK(s.state == SNAKE_READY);
    CHECK(s.length == SNAKE_START_LENGTH);
    CHECK(head(&s).x == SNAKE_COLS / 2 && head(&s).y == SNAKE_ROWS / 2);
    CHECK(snake_occupied(&s, SNAKE_COLS / 2 - SNAKE_START_LENGTH + 1, SNAKE_ROWS / 2));
    CHECK(!snake_occupied(&s, s.food.x, s.food.y));
    CHECK(!snake_update(&s, 10000));
}

static void test_first_turn_starts_and_steps_on_interval(void) {
    snake_t s;
    start(&s);
    snake_turn(&s, SNAKE_UP, 100);
    CHECK(s.state == SNAKE_PLAYING);
    CHECK(!snake_update(&s, 100 + SNAKE_START_STEP_MS - 1));
    CHECK(snake_update(&s, 100 + SNAKE_START_STEP_MS));
    CHECK(head(&s).x == SNAKE_COLS / 2 && head(&s).y == SNAKE_ROWS / 2 - 1);
    CHECK(s.length == SNAKE_START_LENGTH);
    CHECK(!snake_occupied(&s, SNAKE_COLS / 2 - SNAKE_START_LENGTH + 1, SNAKE_ROWS / 2));
}

static void test_reversal_is_ignored(void) {
    snake_t s;
    start(&s);
    snake_turn(&s, SNAKE_LEFT, 0);
    CHECK(s.state == SNAKE_PLAYING);
    step(&s, 0);
    CHECK(s.dir == SNAKE_RIGHT);
    CHECK(s.state == SNAKE_PLAYING);
}

static void test_quick_turns_are_queued(void) {
    snake_t s;
    start(&s);
    snake_turn(&s, SNAKE_UP, 0);
    snake_turn(&s, SNAKE_LEFT, 0);
    uint32_t now = step(&s, 0);
    CHECK(s.dir == SNAKE_UP);
    step(&s, now);
    CHECK(s.dir == SNAKE_LEFT);
    CHECK(head(&s).x == SNAKE_COLS / 2 - 1 && head(&s).y == SNAKE_ROWS / 2 - 1);
}

static void test_wall_ends_round(void) {
    snake_t s;
    start(&s);
    snake_turn(&s, SNAKE_DOWN, 0);
    uint32_t now = 0;
    for (uint8_t i = 0; i < SNAKE_ROWS / 2 - 1; ++i) {
        now = step(&s, now);
    }
    CHECK(s.state == SNAKE_PLAYING);
    now = step(&s, now);
    CHECK(s.state == SNAKE_OVER);
    CHECK(!snake_can_restart(&s, now + SNAKE_RESTART_DELAY_MS - 1));
    CHECK(snake_can_restart(&s, now + SNAKE_RESTART_DELAY_MS));
}

static void test_eating_grows_scores_and_speeds_up(void) {
    snake_t s;
    start(&s);
    s.food = (snake_cell_t){SNAKE_COLS / 2 + 1, SNAKE_ROWS / 2};
    snake_turn(&s, SNAKE_DOWN, 0);
    s.queued_count = 0; // keep heading right
    step(&s, 0);
    CHECK(s.score == 1);
    CHECK(s.length == SNAKE_START_LENGTH + 1);
    CHECK(snake_step_ms(&s) == SNAKE_START_STEP_MS - SNAKE_SPEEDUP_MS);
    CHECK(!snake_occupied(&s, s.food.x, s.food.y));
    s.score = 1000;
    CHECK(snake_step_ms(&s) == SNAKE_MIN_STEP_MS);
}

static void test_self_collision_but_tail_chasing_is_legal(void) {
    snake_t s;
    snake_init(&s, 1);
    s.food = (snake_cell_t){0, 0};
    // Length 4 in a 2x2 square: tail chasing should be fine forever.
    snake_turn(&s, SNAKE_DOWN, 0);
    snake_turn(&s, SNAKE_LEFT, 0);
    uint32_t now = step(&s, 0);
    now          = step(&s, now);
    snake_turn(&s, SNAKE_UP, now);
    now = step(&s, now);
    CHECK(s.state == SNAKE_PLAYING);
    snake_turn(&s, SNAKE_RIGHT, now);
    now = step(&s, now);
    CHECK(s.state == SNAKE_PLAYING);

    // A fifth segment means the tail doesn't clear in time.
    snake_init(&s, 1);
    s.food = (snake_cell_t){SNAKE_COLS / 2 + 1, SNAKE_ROWS / 2};
    snake_turn(&s, SNAKE_DOWN, 0);
    s.queued_count = 0;
    now            = step(&s, 0); // eat, length 5
    s.food         = (snake_cell_t){0, 0};
    snake_turn(&s, SNAKE_DOWN, now);
    snake_turn(&s, SNAKE_LEFT, now);
    now = step(&s, now);
    now = step(&s, now);
    snake_turn(&s, SNAKE_UP, now);
    uint16_t length = s.length;
    now             = step(&s, now);
    CHECK(s.state == SNAKE_OVER);
    CHECK(s.length == length);
}

static void test_food_avoids_body(void) {
    for (uint32_t seed = 1; seed < 200; ++seed) {
        snake_t s;
        snake_init(&s, seed);
        CHECK(!snake_occupied(&s, s.food.x, s.food.y));
        CHECK(s.food.x < SNAKE_COLS && s.food.y < SNAKE_ROWS);
    }
}

static void test_page_bytes(void) {
    snake_t s;
    start(&s);
    uint8_t col  = SNAKE_COLS / 2;
    uint8_t page = SNAKE_ROWS / 2 / 2;
    // Head sits in the upper half of its page (even row).
    CHECK(snake_page_byte(&s, col * SNAKE_CELL_PX, page) == 0x07);
    CHECK(snake_page_byte(&s, col * SNAKE_CELL_PX + 3, page) == 0x00);
    CHECK(snake_page_byte(&s, (col + 1) * SNAKE_CELL_PX, page) == 0x00);
    s.food = (snake_cell_t){0, 1};
    CHECK(snake_page_byte(&s, 1, 0) == 0x50);
    CHECK(snake_page_byte(&s, 0, 0) == 0x20);
}

int main(void) {
    test_init();
    test_first_turn_starts_and_steps_on_interval();
    test_reversal_is_ignored();
    test_quick_turns_are_queued();
    test_wall_ends_round();
    test_eating_grows_scores_and_speeds_up();
    test_self_collision_but_tail_chasing_is_legal();
    test_food_avoids_body();
    test_page_bytes();
    return CHECK_DONE();
}
