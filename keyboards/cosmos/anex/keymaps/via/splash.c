#include "splash.h"

splash_state_t splash_update(splash_t *splash, uint32_t now) {
    if (splash->done) {
        return SPLASH_DONE;
    }
    if (!splash->started) {
        splash->started = true;
        splash->start   = now;
    }
    if (now - splash->start < SPLASH_DURATION_MS) {
        return SPLASH_SHOWING;
    }
    splash->done = true;
    return SPLASH_FINISHED;
}
