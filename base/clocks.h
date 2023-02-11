#pragma once
#include <stdint.h>
#include <time.h>

namespace base_clocks {

constexpr unsigned NS_PER_SECOND = 1000000000;

typedef uint64_t timed_ns_t;
typedef uint64_t timed_us_t;
typedef uint64_t timed_ms_t;
typedef unsigned timed_seconds_t;

static inline bool sleep_ns(timed_ns_t ns) {
    timespec ts;
    ts.tv_nsec = ns % NS_PER_SECOND;
    ts.tv_sec = ns / NS_PER_SECOND;
    return 0 == ::clock_nanosleep(CLOCK_REALTIME, 0, &ts, 0);
}

static inline bool sleep_us(timed_us_t us) {
    return sleep_ns(us * 1000);
}
static inline bool sleep_ms(timed_ms_t ms) {
    return sleep_us(ms * 1000);
}
static inline bool sleep_seconds(timed_seconds_t n) {
    return sleep_ms(n * 1000);
}

//
//  Use nanosecond resolution of REALTIME clock.
//

namespace clock_realtime {

static inline timed_ns_t time_get() {
    struct timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    timed_ns_t t = ts.tv_sec;
    return (NS_PER_SECOND * t) + ts.tv_nsec;
}

static inline timed_ns_t ns_from_us(timed_us_t v) {
    return 1000 * v;
}
static inline timed_ns_t ns_from_ms(timed_ms_t v) {
    return ns_from_us(1000 * v);
}
static inline timed_ns_t ns_from_seconds(timed_seconds_t v) {
    return ns_from_ms(1000 * v);
}
static inline timed_ns_t ns_from_seconds(double v) {
    return v / NS_PER_SECOND;
}
static inline double seconds_from_ns(timed_ns_t v) {
    return ((double) v) / NS_PER_SECOND;
}

};  // namespace clock_realtime

};  // namespace base_clocks
