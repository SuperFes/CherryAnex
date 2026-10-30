#include "graph.h"
#include "check.h"

static void test_empty_graph_reads_zero(void) {
    graph_t graph = {0};
    CHECK(graph_at(&graph, 0) == 0);
    CHECK(graph_max(&graph) == 0);
}

static void test_newest_sample_is_age_zero(void) {
    graph_t graph = {0};
    graph_push(&graph, 10);
    graph_push(&graph, 20);
    CHECK(graph_at(&graph, 0) == 20);
    CHECK(graph_at(&graph, 1) == 10);
    CHECK(graph_at(&graph, 2) == 0);
}

static void test_wraps_after_capacity(void) {
    graph_t graph = {0};
    for (unsigned i = 0; i < GRAPH_SAMPLES + 5; ++i) {
        graph_push(&graph, (uint8_t)i);
    }
    CHECK(graph.count == GRAPH_SAMPLES);
    CHECK(graph_at(&graph, 0) == GRAPH_SAMPLES + 4);
    CHECK(graph_at(&graph, GRAPH_SAMPLES - 1) == 5);
}

static void test_max_only_sees_retained_samples(void) {
    graph_t graph = {0};
    graph_push(&graph, 200);
    for (unsigned i = 0; i < GRAPH_SAMPLES; ++i) {
        graph_push(&graph, 7);
    }
    CHECK(graph_max(&graph) == 7);
}

static void test_bar_height(void) {
    CHECK(graph_bar_height(0, 100, 32) == 0);
    CHECK(graph_bar_height(1, 100, 32) == 1);
    CHECK(graph_bar_height(50, 100, 32) == 16);
    CHECK(graph_bar_height(100, 100, 32) == 32);
    CHECK(graph_bar_height(250, 100, 32) == 32);
    CHECK(graph_bar_height(5, 0, 32) == 0);
}

static void test_page_byte_fills_from_bottom(void) {
    // Four pages: page 3 is the bottom.
    CHECK(graph_page_byte(0, 3, 4) == 0x00);
    CHECK(graph_page_byte(1, 3, 4) == 0x80);
    CHECK(graph_page_byte(3, 3, 4) == 0xE0);
    CHECK(graph_page_byte(8, 3, 4) == 0xFF);
    CHECK(graph_page_byte(8, 2, 4) == 0x00);
    CHECK(graph_page_byte(10, 2, 4) == 0xC0);
    CHECK(graph_page_byte(32, 0, 4) == 0xFF);
    CHECK(graph_page_byte(31, 0, 4) == 0xFE);
}

int main(void) {
    test_empty_graph_reads_zero();
    test_newest_sample_is_age_zero();
    test_wraps_after_capacity();
    test_max_only_sees_retained_samples();
    test_bar_height();
    test_page_byte_fills_from_bottom();
    return CHECK_DONE();
}
