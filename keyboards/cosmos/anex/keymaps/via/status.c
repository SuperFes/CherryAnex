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
