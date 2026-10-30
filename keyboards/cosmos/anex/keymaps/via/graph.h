#pragma once

// Rolling history of small values (WPM samples) drawn as a bar graph in
// SSD1306 page bytes. Free of QMK headers so it can be unit tested on the host.

#include <stdint.h>

#define GRAPH_SAMPLES 128

typedef struct {
    uint8_t samples[GRAPH_SAMPLES];
    uint8_t head; // next slot to write
    uint8_t count;
} graph_t;

void graph_push(graph_t *graph, uint8_t value);

// age 0 is the newest sample; ages at or past count read as 0.
uint8_t graph_at(const graph_t *graph, uint8_t age);

uint8_t graph_max(const graph_t *graph);

// Bar height in pixels for value, where full_scale fills height.
uint8_t graph_bar_height(uint8_t value, uint8_t full_scale, uint8_t height);

// Byte for one column of page `page` (0 = top) in a graph `pages` tall, with
// a bar `bar_height` pixels tall growing up from the bottom.
uint8_t graph_page_byte(uint8_t bar_height, uint8_t page, uint8_t pages);
