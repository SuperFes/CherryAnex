#pragma once

// Minimal assertion helpers for host-side tests of QMK-independent code.

#include <stdio.h>
#include <string.h>

static int check_failures = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        if (!(cond)) {                                                 \
            fprintf(stderr, "%s:%d: CHECK(%s)\n", __FILE__, __LINE__, #cond); \
            ++check_failures;                                          \
        }                                                              \
    } while (0)

#define CHECK_STR(actual, expected)                                                        \
    do {                                                                                   \
        const char *a_ = (actual), *e_ = (expected);                                       \
        if (strcmp(a_, e_) != 0) {                                                         \
            fprintf(stderr, "%s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, a_, e_);      \
            ++check_failures;                                                              \
        }                                                                                  \
    } while (0)

#define CHECK_DONE() (check_failures ? (fprintf(stderr, "%d failed\n", check_failures), 1) : 0)
