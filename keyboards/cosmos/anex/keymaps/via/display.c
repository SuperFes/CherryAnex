#include QMK_KEYBOARD_H

#include <string.h>

#include "logo.h"
#include "status.h"
#include "graph.h"
#include "pet.h"
#include "pet_sprites.h"
#include "petfx_sprites.h"
#include "reactions.h"
#include "runner_sprites.h"
#include "splash.h"
#include "stats.h"
#include "game.h"
#include "pc.h"
#include "screensaver.h"

// 128 samples at this interval is about a minute of history.
#define WPM_SAMPLE_INTERVAL_MS 500
// Graph occupies the bottom four pages (32px).
#define WPM_GRAPH_FIRST_PAGE 4
#define WPM_GRAPH_PAGES 4
// Floor for the graph's full scale so slow typing doesn't look frantic.
#define WPM_GRAPH_MIN_SCALE 40
// Pet walks along the bottom four pages.
#define PET_FIRST_PAGE 4
// Redraw rate while the pet is mid-reaction.
#define PET_FRAME_MS 40

// Screensaver frame rate, about 20 fps.
#define SAVER_FRAME_MS 50

// Shared by the scenes that place sprites freely; only one shows at a time.
static canvas_t canvas;

// Set when the screen was cleared, so parts that only draw on change redraw.
static bool redraw = true;

static void write_status_line(const status_line_t *line) {
    // 21 columns fill the row, so the cursor wraps to the next line by itself.
    for (uint8_t col = 0; col < STATUS_COLS; ++col) {
        oled_write_char(line->text[col], line->invert & (1UL << col));
    }
}

// NULL while the anex-host app isn't running.
static const status_pc_t *pc_stats(void) {
    static status_pc_t stats;
    const pc_view_t   *pc = pc_view();

    if (!pc->connected) {
        return NULL;
    }
    stats = (status_pc_t){.hour = pc->hour, .minute = pc->minute, .cpu = pc->cpu, .ram = pc->ram};
    return &stats;
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
    status_lock_line(&line, (status_locks_t){.caps = leds.caps_lock, .num = leds.num_lock, .scroll = leds.scroll_lock}, pc_stats());
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
    status_line_t   line;
    uint8_t         wpm = get_current_wpm();

    if (wpm > peak) {
        peak = wpm;
    }
    status_wpm_line(&line, wpm, peak, pc_stats());
    oled_set_cursor(0, 3);
    write_status_line(&line);

    bool sample = timer_elapsed32(last_sample) >= WPM_SAMPLE_INTERVAL_MS;
    if (sample) {
        last_sample = timer_read32();
        graph_push(&history, wpm);
    }
    if (sample || redraw) {
        render_wpm_graph(&history);
    }
}

static void render_pet(const pet_t *pet, uint32_t now) {
    pet_pose_t pose = pet_pose(pet, now);
    int16_t    top  = PET_FIRST_PAGE * 8;

    canvas_clear(&canvas);
    // Also drawn a screen-width either side, so the pet wraps around the edges.
    for (int16_t wrap = -PET_SCREEN_WIDTH; wrap <= PET_SCREEN_WIDTH; wrap += PET_SCREEN_WIDTH) {
        int16_t x = pet->x + pose.dx + wrap;
        canvas_blit(&canvas, (const uint8_t *)pet_sprites[pose.sprite], PET_SPRITE_WIDTH, PET_SPRITE_HEIGHT, x, top + pose.dy);
        if (pose.fx >= 0) {
            canvas_blit(&canvas, (const uint8_t *)petfx_sprites[pose.fx], PETFX_SPRITE_WIDTH, PETFX_SPRITE_HEIGHT, x + pose.fx_x, top + pose.fx_y);
        }
    }
    // Only the pet's pages; the text rows above belong to the status strip.
    for (uint16_t i = PET_FIRST_PAGE * OLED_DISPLAY_WIDTH; i < CANVAS_BYTES; ++i) {
        oled_write_raw_byte(canvas.bytes[i], i);
    }
}

static void render_right(void) {
    static pet_t            pet;
    static uint8_t          seen_reaction;
    static uint8_t          seen_sweat;
    static uint32_t         last_draw;
    status_line_t           line;
    const keycount_t       *count    = stats_keycount();
    const game_view_t      *game     = game_view();
    const reactions_view_t *reaction = reactions_view();
    uint32_t                now      = timer_read32();
    bool                    changed  = redraw;

    if (game->active) {
        status_score_line(&line, game->score, game->best);
    } else {
        status_count_line(&line, count->total, count->session);
    }
    oled_set_cursor(0, 3);
    write_status_line(&line);

    if (reaction->seq != seen_reaction) {
        seen_reaction = reaction->seq;
        pet_react(&pet, reaction->reaction, now);
        changed = true;
    }
    if (reaction->sweat_seq != seen_sweat) {
        seen_sweat = reaction->sweat_seq;
        pet_sweat(&pet, now);
        changed = true;
    }
    // Mods and LEDs are synced from the USB half, so either half can show them.
    bool sneak = get_mods() & MOD_MASK_CTRL;
    bool panic = host_keyboard_led_state().caps_lock;
    if (sneak != pet.sneak || panic != pet.panic) {
        pet_hold(&pet, sneak, panic);
        changed = true;
    }
    changed |= pet_update(&pet, get_current_wpm(), last_input_activity_elapsed(), now);
    changed |= pet_animating(&pet, now) && timer_elapsed32(last_draw) >= PET_FRAME_MS;
    if (changed) {
        render_pet(&pet, now);
        last_draw = now;
    }
}

static void write_canvas(void) {
    for (uint16_t i = 0; i < CANVAS_BYTES; ++i) {
        oled_write_raw_byte(canvas.bytes[i], i);
    }
}

// Text over a game scene: a hint before the first move, GAME OVER after.
static void write_game_banner(uint8_t state, const char *hint) {
    if (state == SNAKE_READY) {
        oled_set_cursor((OLED_DISPLAY_WIDTH / OLED_FONT_WIDTH - strlen(hint)) / 2, 1);
        oled_write(hint, false);
    } else if (state == SNAKE_OVER) {
        oled_set_cursor(6, 1);
        oled_write_P(PSTR("GAME OVER"), true);
    }
}

static void render_menu(void) {
    const game_view_t *game = game_view();

    oled_clear();
    oled_set_cursor(5, 1);
    oled_write_P(PSTR("PICK A GAME"), false);
    for (uint8_t kind = 0; kind < GAME_COUNT; ++kind) {
        bool selected = kind == game->kind;
        oled_set_cursor(6, 3 + kind * 2);
        oled_write(selected ? ">" : " ", false);
        oled_write(game_name(kind), selected);
    }
}

static void render_snake(void) {
    const snake_t *snake = game_snake();

    for (uint8_t page = 0; page < OLED_DISPLAY_HEIGHT / 8; ++page) {
        for (uint8_t x = 0; x < OLED_DISPLAY_WIDTH; ++x) {
            oled_write_raw_byte(snake_page_byte(snake, x, page), page * OLED_DISPLAY_WIDTH + x);
        }
    }
    write_game_banner(snake->state, "ARROWS OR WASD: START");
}

static void render_runner(void) {
    const runner_t      *runner = game_runner();
    const uint8_t *const pet[3] = {
        (const uint8_t *)pet_sprites[PET_SPRITE_RUN_1],
        (const uint8_t *)pet_sprites[PET_SPRITE_RUN_2],
        (const uint8_t *)pet_sprites[PET_SPRITE_BLINK],
    };
    const uint8_t *obstacles[RUNNER_SPRITE_COUNT];
    for (uint8_t i = 0; i < RUNNER_SPRITE_COUNT; ++i) {
        obstacles[i] = (const uint8_t *)runner_sprites[i];
    }

    runner_draw(&canvas, runner, pet, obstacles);
    write_canvas();
    write_game_banner(runner->state, "SPACE OR UP: JUMP");
}

static void render_game(void) {
    const game_view_t *game = game_view();

    if (game->menu) {
        render_menu();
    } else if (game->kind == GAME_RUNNER) {
        render_runner();
    } else {
        render_snake();
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
    return OLED_ROTATION_0;
}

static void render_saver(void) {
    static uint32_t last_frame;

    if (timer_elapsed32(last_frame) < SAVER_FRAME_MS && !redraw) {
        return;
    }
    last_frame = timer_read32();

    // Both halves draw from the split-synchronised clock so scenes line up.
    uint32_t t    = sync_timer_read32();
    uint8_t  half = is_keyboard_left() ? 0 : 1;
    if (screensaver_view()->style == SAVER_PET) {
        const uint8_t *const frames[2] = {
            (const uint8_t *)pet_sprites[PET_SPRITE_SLEEP_1],
            (const uint8_t *)pet_sprites[PET_SPRITE_SLEEP_2],
        };
        saver_draw_pet(&canvas, t, half, frames);
    } else {
        saver_draw_stars(&canvas, t, half);
    }
    write_canvas();
}

typedef enum {
    SHOW_NORMAL,
    SHOW_GAME,
    SHOW_SAVER,
    SHOW_OFF,
} show_t;

static show_t what_to_show(void) {
    bool gaming = game_view()->active;
    switch (saver_screen(last_input_activity_elapsed(), screensaver_view()->preview, gaming)) {
        case SAVER_SCREEN_OFF:
            return SHOW_OFF;
        case SAVER_SCREEN_SAVER:
            return SHOW_SAVER;
        default:
            // The game runs where the keys are processed.
            return gaming && is_keyboard_master() ? SHOW_GAME : SHOW_NORMAL;
    }
}

bool oled_task_user(void) {
    static show_t showing = SHOW_NORMAL;

    if (render_splash()) {
        return false;
    }

    show_t next = what_to_show();
    if (next != showing) {
        showing = next;
        if (showing == SHOW_OFF) {
            oled_off();
        } else {
            oled_clear();
            redraw = true;
        }
    }

    switch (showing) {
        case SHOW_OFF:
            // Drawing anything would switch the display back on.
            break;
        case SHOW_GAME:
            if (game_take_dirty() || redraw) {
                render_game();
            }
            break;
        case SHOW_SAVER:
            render_saver();
            break;
        default:
            render_status_strip();
            if (is_keyboard_left()) {
                render_left();
            } else {
                render_right();
            }
            break;
    }
    redraw = false;
    return false;
}
