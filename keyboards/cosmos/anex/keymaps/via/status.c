#include "status.h"

#include <string.h>

#define LAYER_TAB_COLS 8

static const char *const layer_names[] = {"MAIN", "SYMBOL", "GAMES", "RGB"};

const char *status_layer_name(uint8_t layer) {
    if (layer < sizeof(layer_names) / sizeof(layer_names[0])) {
        return layer_names[layer];
    }
    return "INSANE!";
}

static void line_clear(status_line_t *line) {
    memset(line->text, ' ', STATUS_COLS);
    line->text[STATUS_COLS] = '\0';
    line->invert            = 0;
}

static void line_put(status_line_t *line, uint8_t col, const char *text, bool invert) {
    for (; *text && col < STATUS_COLS; ++text, ++col) {
        line->text[col] = *text;
        if (invert) {
            line->invert |= 1UL << col;
        }
    }
}

static void line_invert_span(status_line_t *line, uint8_t col, uint8_t count) {
    for (; count && col < STATUS_COLS; --count, ++col) {
        line->invert |= 1UL << col;
    }
}

void status_layer_line(status_line_t *line, uint8_t layer, status_mods_t mods) {
    line_clear(line);
    line_invert_span(line, 0, LAYER_TAB_COLS);
    line_put(line, 1, status_layer_name(layer), true);

    // Rightmost columns, in the usual Ctrl Shift Alt Gui order.
    const uint8_t first = STATUS_COLS - 7;
    line_put(line, first + 0, "C", mods.ctrl);
    line_put(line, first + 2, "S", mods.shift);
    line_put(line, first + 4, "A", mods.alt);
    line_put(line, first + 6, "G", mods.gui);
}

void status_lock_line(status_line_t *line, status_locks_t locks) {
    line_clear(line);
    line_put(line, 0, "CAPS", locks.caps);
    line_put(line, 8, "NUM", locks.num);
    line_put(line, 17, "SCRL", locks.scroll);
}

static uint8_t digits(uint32_t value) {
    uint8_t n = 1;
    for (; value >= 10; value /= 10) {
        ++n;
    }
    return n;
}

// Writes value into width columns, scaling it down until it fits.
static void line_put_count(status_line_t *line, uint8_t col, uint32_t value, uint8_t width) {
    static const char suffixes[] = {'\0', 'k', 'M', 'G'};
    uint8_t           scale      = 0;
    for (; scale < sizeof(suffixes) - 1 && digits(value) + (scale ? 1 : 0) > width; ++scale) {
        value /= 1000;
    }

    char    text[12];
    uint8_t len = digits(value);
    for (uint8_t i = len; i; --i, value /= 10) {
        text[i - 1] = '0' + value % 10;
    }
    if (scale) {
        text[len++] = suffixes[scale];
    }
    text[len] = '\0';
    line_put(line, col, text, false);
}

void status_count_line(status_line_t *line, uint32_t total, uint32_t session) {
    line_clear(line);
    line_put(line, 0, "ALL", false);
    line_put_count(line, 4, total, 8);
    line_put(line, 13, "NOW", false);
    line_put_count(line, 17, session, 4);
}

void status_score_line(status_line_t *line, uint16_t score, uint16_t best) {
    line_clear(line);
    line_put(line, 0, "SCORE", false);
    line_put_count(line, 6, score, 4);
    line_put(line, 12, "BEST", false);
    line_put_count(line, 17, best, 4);
}
