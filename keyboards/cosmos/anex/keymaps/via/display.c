#include QMK_KEYBOARD_H
#include "logo.h"
#include "status.h"
#include "graph.h"
#include "pet.h"
#include "pet_sprites.h"
#include "splash.h"
#include "stats.h"

// 128 samples at this interval is about a minute of history.
#define WPM_SAMPLE_INTERVAL_MS 500
// Graph occupies the bottom four pages (32px).
#define WPM_GRAPH_FIRST_PAGE 4
#define WPM_GRAPH_PAGES 4
// Floor for the graph's full scale so slow typing doesn't look frantic.
#define WPM_GRAPH_MIN_SCALE 40
// Pet walks along the bottom four pages.
#define PET_FIRST_PAGE 4

static void write_status_line(const status_line_t *line) {
    // 21 columns fill the row, so the cursor wraps to the next line by itself.
    for (uint8_t col = 0; col < STATUS_COLS; ++col) {
        oled_write_char(line->text[col], line->invert & (1UL << col));
    }
}

// Two text rows plus a rule; side-specific content starts on row 3.
static void render_status_strip(void) {
    status_line_t line;
    uint8_t       mods = get_mods() | get_oneshot_mods();
    led_t         leds = host_keyboard_led_state();

    oled_set_cursor(0, 0);
    status_layer_line(&line, get_highest_layer(layer_state | default_layer_state),
                      (status_mods_t){
                          .ctrl  = mods & MOD_MASK_CTRL,
                          .shift = mods & MOD_MASK_SHIFT,
                          .alt   = mods & MOD_MASK_ALT,
                          .gui   = mods & MOD_MASK_GUI,
                      });
    write_status_line(&line);
    status_lock_line(&line, (status_locks_t){.caps = leds.caps_lock, .num = leds.num_lock, .scroll = leds.scroll_lock});
    write_status_line(&line);

    for (uint16_t x = 0; x < OLED_DISPLAY_WIDTH; ++x) {
        oled_write_raw_byte(0x08, 2 * OLED_DISPLAY_WIDTH + x);
    }
}

static void render_wpm_graph(const graph_t *history) {
    uint8_t scale = graph_max(history);
    if (scale < WPM_GRAPH_MIN_SCALE) {
        scale = WPM_GRAPH_MIN_SCALE;
    }
    // Newest sample at the right edge.
    for (uint8_t x = 0; x < GRAPH_SAMPLES; ++x) {
        uint8_t height = graph_bar_height(graph_at(history, GRAPH_SAMPLES - 1 - x), scale, WPM_GRAPH_PAGES * 8);
        for (uint8_t page = 0; page < WPM_GRAPH_PAGES; ++page) {
            oled_write_raw_byte(graph_page_byte(height, page, WPM_GRAPH_PAGES), (WPM_GRAPH_FIRST_PAGE + page) * OLED_DISPLAY_WIDTH + x);
        }
    }
}

static void render_left(void) {
    static graph_t  history;
    static uint8_t  peak;
    static uint32_t last_sample;
    uint8_t         wpm = get_current_wpm();

    if (wpm > peak) {
        peak = wpm;
    }
    oled_set_cursor(0, 3);
    oled_write_P(PSTR("WPM "), false);
    oled_write(get_u8_str(wpm, ' '), false);
    oled_write_P(PSTR("     PEAK "), false);
    oled_write(get_u8_str(peak, ' '), false);

    if (timer_elapsed32(last_sample) >= WPM_SAMPLE_INTERVAL_MS) {
        last_sample = timer_read32();
        graph_push(&history, wpm);
        render_wpm_graph(&history);
    }
}

static void render_pet(const pet_t *pet) {
    const char *sprite = pet_sprites[pet_sprite(pet)];

    for (uint8_t x = 0; x < PET_SCREEN_WIDTH; ++x) {
        // Sprite column at this screen column, wrapping off the right edge.
        uint8_t col = (x + PET_SCREEN_WIDTH - pet->x) % PET_SCREEN_WIDTH;
        for (uint8_t page = 0; page < PET_SPRITE_HEIGHT / 8; ++page) {
            uint8_t byte = col < PET_SPRITE_WIDTH ? pgm_read_byte(&sprite[page * PET_SPRITE_WIDTH + col]) : 0;
            oled_write_raw_byte(byte, (PET_FIRST_PAGE + page) * OLED_DISPLAY_WIDTH + x);
        }
    }
}

static void render_right(void) {
    static pet_t       pet;
    static bool        drawn;
    status_line_t      line;
    const keycount_t  *count = stats_keycount();

    status_count_line(&line, count->total, count->session);
    oled_set_cursor(0, 3);
    write_status_line(&line);

    if (pet_update(&pet, get_current_wpm(), last_input_activity_elapsed(), timer_read32()) || !drawn) {
        render_pet(&pet);
        drawn = true;
    }
}

// Returns true while the boot logo still owns the screen.
static bool render_splash(void) {
    static splash_t splash;

    switch (splash_update(&splash, timer_read32())) {
        case SPLASH_SHOWING:
            oled_write_raw_P(anex_logo, sizeof(anex_logo));
            return true;
        case SPLASH_FINISHED:
            oled_clear();
            return false;
        default:
            return false;
    }
}

// Mounted with the panel's header at the bottom.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;
}

bool oled_task_user(void) {
    if (render_splash()) {
        return false;
    }
    render_status_strip();
    if (is_keyboard_left()) {
        render_left();
    } else {
        render_right();
    }
    return false;
}
