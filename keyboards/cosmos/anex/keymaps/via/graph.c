#include "graph.h"

void graph_push(graph_t *graph, uint8_t value) {
    graph->samples[graph->head] = value;
    graph->head                 = (graph->head + 1) % GRAPH_SAMPLES;
    if (graph->count < GRAPH_SAMPLES) {
        ++graph->count;
    }
}

uint8_t graph_at(const graph_t *graph, uint8_t age) {
    if (age >= graph->count) {
        return 0;
    }
    return graph->samples[(graph->head + GRAPH_SAMPLES - 1 - age) % GRAPH_SAMPLES];
}

uint8_t graph_max(const graph_t *graph) {
    uint8_t max = 0;
    for (uint8_t age = 0; age < graph->count; ++age) {
        uint8_t value = graph_at(graph, age);
        if (value > max) {
            max = value;
        }
    }
    return max;
}

uint8_t graph_bar_height(uint8_t value, uint8_t full_scale, uint8_t height) {
    if (full_scale == 0 || value == 0) {
        return 0;
    }
    if (value >= full_scale) {
        return height;
    }
    // Round up so any typing at all shows at least one pixel.
    return (uint8_t)(((uint16_t)value * height + full_scale - 1) / full_scale);
}

uint8_t graph_page_byte(uint8_t bar_height, uint8_t page, uint8_t pages) {
    // Pixel rows covered by this page, counted up from the bottom of the graph.
    uint8_t page_bottom = (uint8_t)((pages - 1 - page) * 8);

    if (bar_height <= page_bottom) {
        return 0;
    }
    uint8_t filled = bar_height - page_bottom;
    if (filled >= 8) {
        return 0xFF;
    }
    // Bit 7 is the bottom row of a page.
    return (uint8_t)(0xFF << (8 - filled));
}
