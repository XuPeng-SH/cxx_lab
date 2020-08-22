#include "monotime.h"

#include <time.h>

CoarseMonoClock::time_point CoarseMonoClock::now() {
    struct timespec ts;
    /* if (clock_gettime(CLOCK_MONOTONIC_COARSE, &ts) == 0) */
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    auto nanos = static_cast<int64_t>(ts.tv_sec) * 1000 * 1000 * 1000 + ts.tv_nsec;
    return time_point(duration(nanos));
}
