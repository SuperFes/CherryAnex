// Replays the display scenes on the PC for tools/render_gifs.py. Drives the
// firmware's QMK-free modules with scripted input; the QMK glue in display.c
// is mirrored here, so keep the two in step. Each scene is written to
// <out>/<scene>.bin as frames of: u16 delay_ms (host order), 1024 bytes left
// OLED, 1024 bytes right OLED, both in SSD1306 page format.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "progmem.h"
#include "glcdfont.c"

#include "canvas.h"
#include "graph.h"
#include "logo.h"
#include "pet.h"
#include "pet_sprites.h"
#include "petfx_sprites.h"
#include "runner.h"
#include "runner_sprites.h"
#include "saver.h"
#include "snake.h"
#include "status.h"

#define W 128
#define BYTES 1024

typedef struct {
    uint8_t buf[BYTES];
    uint8_t col, row;
} oled_t;

static void o_clear(oled_t *o) {
    memset(o->buf, 0, BYTES);
    o->col = o->row = 0;
}
static void o_cursor(oled_t *o, uint8_t col, uint8_t row) {
    o->col = col;
    o->row = row;
}
static void o_char(oled_t *o, char c, bool inv) {
    uint8_t *dst = &o->buf[o->row * W + o->col * 6];
    for (int i = 0; i < 6; ++i) {
        uint8_t b = font[(uint8_t)c * 6 + i];
        dst[i]    = inv ? ~b : b;
    }
    if (++o->col >= 21) {
        o->col = 0;
        o->row = (o->row + 1) % 8;
    }
}
static void o_write(oled_t *o, const char *s, bool inv) {
    while (*s) o_char(o, *s++, inv);
}
static void o_line(oled_t *o, const status_line_t *l) {
    for (int c = 0; c < STATUS_COLS; ++c) o_char(o, l->text[c], l->invert & (1UL << c));
}
static void o_canvas(oled_t *o, const canvas_t *c, int first_page) {
    memcpy(&o->buf[first_page * W], &c->bytes[first_page * W], BYTES - first_page * W);
}

static FILE       *out;
static const char *out_dir;
static void emit(const oled_t *l, const oled_t *r, uint16_t delay) {
    fwrite(&delay, 2, 1, out);
    fwrite(l->buf, BYTES, 1, out);
    fwrite(r->buf, BYTES, 1, out);
}
static void open_scene(const char *name) {
    char path[4096];
    snprintf(path, sizeof path, "%s/%s.bin", out_dir, name);
    out = fopen(path, "wb");
    if (!out) {
        perror(path);
        exit(1);
    }
}

// Everything the normal (non-game, non-saver) screens read from QMK.
typedef struct {
    uint8_t        layer;
    status_mods_t  mods;
    status_locks_t locks;
    bool           pc;
    status_pc_t    pcs;
    uint8_t        wpm;
    uint32_t       total, session;
    uint32_t       idle_ms;
} kb_t;

typedef struct {
    graph_t  history;
    uint8_t  peak;
    uint32_t last_sample;
    pet_t    pet;
    canvas_t canvas;
} normal_t;

static void status_strip(oled_t *o, const kb_t *kb) {
    status_line_t line;
    o_cursor(o, 0, 0);
    status_layer_line(&line, kb->layer, kb->mods);
    o_line(o, &line);
    status_lock_line(&line, kb->locks, kb->pc ? &kb->pcs : NULL);
    o_line(o, &line);
    for (int x = 0; x < W; ++x) o->buf[2 * W + x] = 0x08;
}

static void render_left(oled_t *o, normal_t *n, const kb_t *kb, uint32_t now) {
    status_line_t line;
    status_strip(o, kb);
    if (kb->wpm > n->peak) n->peak = kb->wpm;
    status_wpm_line(&line, kb->wpm, n->peak, kb->pc ? &kb->pcs : NULL);
    o_cursor(o, 0, 3);
    o_line(o, &line);
    if (now - n->last_sample >= 500) {
        n->last_sample = now;
        graph_push(&n->history, kb->wpm);
    }
    uint8_t scale = graph_max(&n->history);
    if (scale < 40) scale = 40;
    for (int x = 0; x < GRAPH_SAMPLES; ++x) {
        uint8_t h = graph_bar_height(graph_at(&n->history, GRAPH_SAMPLES - 1 - x), scale, 32);
        for (int p = 0; p < 4; ++p) o->buf[(4 + p) * W + x] = graph_page_byte(h, p, 4);
    }
}

static void draw_pet(oled_t *o, normal_t *n, uint32_t now) {
    pet_pose_t pose = pet_pose(&n->pet, now);
    canvas_clear(&n->canvas);
    for (int wrap = -PET_SCREEN_WIDTH; wrap <= PET_SCREEN_WIDTH; wrap += PET_SCREEN_WIDTH) {
        int x = n->pet.x + pose.dx + wrap;
        canvas_blit(&n->canvas, (const uint8_t *)pet_sprites[pose.sprite], 32, 32, x, 32 + pose.dy);
        if (pose.fx >= 0) canvas_blit(&n->canvas, (const uint8_t *)petfx_sprites[pose.fx], PETFX_SPRITE_WIDTH, PETFX_SPRITE_HEIGHT, x + pose.fx_x, 32 + pose.fx_y);
    }
    o_canvas(o, &n->canvas, 4);
}

static void render_right(oled_t *o, normal_t *n, const kb_t *kb, uint32_t now) {
    status_line_t line;
    status_strip(o, kb);
    status_count_line(&line, kb->total, kb->session);
    o_cursor(o, 0, 3);
    o_line(o, &line);
    if (kb->mods.ctrl != n->pet.sneak || kb->locks.caps != n->pet.panic) pet_hold(&n->pet, kb->mods.ctrl, kb->locks.caps);
    pet_update(&n->pet, kb->wpm, kb->idle_ms, now);
    draw_pet(o, n, now);
}

#define FRAME_MS 40

static oled_t   L, R;
static normal_t nl, nr;
static kb_t     kb;
static uint32_t now;

static void reset_normal(void) {
    memset(&nl, 0, sizeof nl);
    memset(&nr, 0, sizeof nr);
    o_clear(&L);
    o_clear(&R);
    kb = (kb_t){.pc = true, .pcs = {14, 32, 12, 41}, .total = 1284312, .session = 2210};
}

static void normal_frame(void) {
    render_left(&L, &nl, &kb, now);
    render_right(&R, &nr, &kb, now);
    emit(&L, &R, FRAME_MS);
    now += FRAME_MS;
    kb.idle_ms += FRAME_MS;
}

// Runs normal frames for ms with keys arriving at the given rate.
static void typing(uint32_t ms, uint8_t wpm, int space_every) {
    uint32_t key_ms = wpm ? 12000 / wpm : 0; // 5 chars per word
    uint32_t next   = now;
    int      keys   = 0;
    for (uint32_t end = now + ms; now < end;) {
        kb.wpm = wpm;
        if (wpm && now >= next) {
            next += key_ms;
            kb.total++;
            kb.session++;
            kb.idle_ms = 0;
            if (space_every && ++keys % space_every == 0) pet_react(&nr.pet, PET_REACT_HOP, now);
        }
        normal_frame();
    }
}

static void press(void) {
    kb.total++;
    kb.session++;
    kb.idle_ms = 0;
}

static void scene_boot(void) {
    open_scene("boot");
    reset_normal();
    memcpy(L.buf, anex_logo, BYTES);
    memcpy(R.buf, anex_logo, BYTES);
    for (int i = 0; i < 50; ++i) emit(&L, &R, FRAME_MS);
    o_clear(&L);
    o_clear(&R);
    kb.idle_ms = 0;
    typing(3000, 0, 0);
    fclose(out);
}

static void scene_typing(void) {
    open_scene("typing");
    reset_normal();
    static const uint8_t ramp[] = {0, 18, 26, 34, 45, 58, 72, 81, 76, 64, 50, 30, 12, 0};
    typing(1500, 0, 0);
    for (size_t i = 0; i < sizeof ramp; ++i) typing(1200, ramp[i], 6);
    typing(1500, 0, 0);
    fclose(out);
}

static void scene_sneak_panic(void) {
    open_scene("sneak_panic");
    reset_normal();
    nr.pet.x = 40;
    typing(800, 0, 0);
    kb.mods.ctrl = true;
    typing(3000, 25, 0);
    kb.mods.ctrl = false;
    typing(800, 25, 0);
    kb.locks.caps = true;
    typing(2500, 0, 0);
    kb.locks.caps = false;
    typing(1000, 0, 0);
    fclose(out);
}

static void scene_backspace(void) {
    open_scene("backspace");
    reset_normal();
    nr.pet.x = 60;
    typing(1000, 0, 0);
    for (int i = 0; i < 7; ++i) {
        press();
        pet_react(&nr.pet, PET_REACT_BACKSTEP, now);
        if (i == 4) pet_sweat(&nr.pet, now);
        typing(280, 0, 0);
    }
    typing(1800, 0, 0);
    fclose(out);
}

static void scene_swears(void) {
    open_scene("swears");
    reset_normal();
    nr.pet.x = 40;
    typing(800, 0, 0);
    for (pet_reaction_t r = PET_REACT_GRAWLIX; r <= PET_REACT_FAINT; ++r) {
        press();
        pet_react(&nr.pet, r, now);
        typing(2900, 0, 0);
    }
    fclose(out);
}

static void saver_scene(const char *name, saver_style_t style, uint32_t t0, uint32_t ms) {
    canvas_t             c;
    const uint8_t *const frames[2] = {(const uint8_t *)pet_sprites[PET_SPRITE_SLEEP_1], (const uint8_t *)pet_sprites[PET_SPRITE_SLEEP_2]};
    open_scene(name);
    for (uint32_t t = t0; t < t0 + ms; t += 50) {
        for (uint8_t half = 0; half < 2; ++half) {
            if (style == SAVER_PET)
                saver_draw_pet(&c, t, half, frames);
            else
                saver_draw_stars(&c, t, half);
            o_canvas(half ? &R : &L, &c, 0);
        }
        emit(&L, &R, 50);
    }
    fclose(out);
}

static void game_right(uint16_t score, uint16_t best);

static void game_menu(int frames_each) {
    for (int sel = 0; sel < 2; ++sel) {
        for (int f = 0; f < frames_each; ++f) {
            o_clear(&L);
            o_cursor(&L, 5, 1);
            o_write(&L, "PICK A GAME", false);
            for (int k = 0; k < 2; ++k) {
                o_cursor(&L, 6, 3 + k * 2);
                o_write(&L, k == sel ? ">" : " ", false);
                o_write(&L, k ? "RUNNER" : "SNAKE", k == sel);
            }
            game_right(0, 0);
            emit(&L, &R, FRAME_MS);
            now += FRAME_MS;
        }
    }
}

static void banner(uint8_t state, const char *hint) {
    if (state == SNAKE_READY) {
        o_cursor(&L, (21 - strlen(hint)) / 2, 1);
        o_write(&L, hint, false);
    } else if (state == SNAKE_OVER) {
        o_cursor(&L, 6, 1);
        o_write(&L, "GAME OVER", true);
    }
}

// Right half while the left runs a game: score line over the pet.
static void game_right(uint16_t score, uint16_t best) {
    status_line_t line;
    status_strip(&R, &kb);
    status_score_line(&line, score, best);
    o_cursor(&R, 0, 3);
    o_line(&R, &line);
    pet_update(&nr.pet, kb.wpm, kb.idle_ms, now);
    draw_pet(&R, &nr, now);
}

static void scene_runner(void) {
    runner_t             r;
    canvas_t             c;
    const uint8_t *const pet[3] = {(const uint8_t *)pet_sprites[PET_SPRITE_RUN_1], (const uint8_t *)pet_sprites[PET_SPRITE_RUN_2], (const uint8_t *)pet_sprites[PET_SPRITE_BLINK]};
    const uint8_t       *obs[RUNNER_SPRITE_COUNT];
    for (int i = 0; i < RUNNER_SPRITE_COUNT; ++i) obs[i] = (const uint8_t *)runner_sprites[i];

    open_scene("runner");
    reset_normal();
    kb.layer = 2;
    game_menu(12);
    runner_init(&r, 7);
    r.last_tick = now;
    uint32_t start = now, bot_until = now + 14000, end = 0;
    while (!end || now < end) {
        if (now - start == 1000) runner_jump(&r, now);
        if (r.state == RUNNER_PLAYING && now < bot_until) {
            runner_box_t p = runner_pet_box(&r);
            for (int i = 0; i < RUNNER_MAX_OBSTACLES; ++i) {
                if (!r.obstacles[i].used) continue;
                int16_t ahead = runner_obstacle_box(&r.obstacles[i]).left - p.right;
                if (ahead >= 0 && ahead <= r.speed * 5 / RUNNER_FP) runner_jump(&r, now);
            }
        }
        runner_update(&r, now);
        if (r.state == RUNNER_OVER && !end) end = now + 2500;
        runner_draw(&c, &r, pet, obs);
        o_canvas(&L, &c, 0);
        banner(r.state, "SPACE OR UP: JUMP");
        game_right(r.score, 412);
        emit(&L, &R, FRAME_MS);
        now += FRAME_MS;
    }
    fclose(out);
}

// BFS to the food; falls back to any move that doesn't die.
static int snake_bot(const snake_t *s) {
    static const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
    static int16_t   from[SNAKE_CELLS];
    static uint16_t  q[SNAKE_CELLS];
    snake_cell_t     h = s->body[s->head];
    memset(from, -1, sizeof from);
    int qh = 0, qt = 0;
    q[qt++]                     = h.y * SNAKE_COLS + h.x;
    from[h.y * SNAKE_COLS + h.x] = -2;
    while (qh < qt) {
        int cur = q[qh++], cx = cur % SNAKE_COLS, cy = cur / SNAKE_COLS;
        if (cx == s->food.x && cy == s->food.y) {
            while (from[cur] >= 0 && from[from[cur]] != -2) cur = from[cur];
            int fx = cur % SNAKE_COLS, fy = cur / SNAKE_COLS;
            for (int d = 0; d < 4; ++d)
                if (h.x + dx[d] == fx && h.y + dy[d] == fy) return d;
        }
        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx[d], ny = cy + dy[d];
            if (nx < 0 || ny < 0 || nx >= SNAKE_COLS || ny >= SNAKE_ROWS) continue;
            int ni = ny * SNAKE_COLS + nx;
            if (from[ni] != -1 || snake_occupied(s, nx, ny)) continue;
            from[ni] = cur;
            q[qt++]  = ni;
        }
    }
    for (int d = 0; d < 4; ++d) {
        int nx = h.x + dx[d], ny = h.y + dy[d];
        if (nx >= 0 && ny >= 0 && nx < SNAKE_COLS && ny < SNAKE_ROWS && !snake_occupied(s, nx, ny)) return d;
    }
    return s->dir;
}

static void scene_snake(void) {
    static snake_t s;
    open_scene("snake");
    reset_normal();
    kb.layer = 2;
    game_menu(12);
    snake_init(&s, 3);
    uint32_t start = now, bot_until = now + 16000, end = 0;
    while (!end || now < end) {
        if (now - start >= 1000) {
            if (s.state == SNAKE_READY) snake_turn(&s, SNAKE_UP, now);
            // Planned one step ahead so the turn lands on the next move.
            if (s.state == SNAKE_PLAYING && !s.queued_count && now - s.last_step + FRAME_MS >= snake_step_ms(&s)) {
                snake_dir_t d = now < bot_until ? (snake_dir_t)snake_bot(&s) : s.dir;
                snake_turn(&s, d, now);
            }
        }
        snake_update(&s, now);
        if (s.state == SNAKE_OVER && !end) end = now + 2500;
        for (int p = 0; p < 8; ++p)
            for (int x = 0; x < W; ++x) L.buf[p * W + x] = snake_page_byte(&s, x, p);
        banner(s.state, "ARROWS OR WASD: START");
        game_right(s.score, 57);
        emit(&L, &R, FRAME_MS);
        now += FRAME_MS;
    }
    fclose(out);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s OUT_DIR\n", argv[0]);
        return 2;
    }
    out_dir = argv[1];
    scene_boot();
    scene_typing();
    scene_sneak_panic();
    scene_backspace();
    scene_swears();
    saver_scene("saver_stars", SAVER_STARS, 0, 8000);
    saver_scene("saver_pet", SAVER_PET, 0, 20000);
    scene_runner();
    scene_snake();
    return 0;
}
