/** @file pp_time.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/pp_time.hpp>
#include <chrono>
#include <cstdio>

namespace xo::pp {
    void
    put_hms_msec(PpSink & sink, xo::time::nanos dt)
    {
        auto hms = std::chrono::hh_mm_ss(dt);
        int h = static_cast<int>(hms.hours().count());
        int m = static_cast<int>(hms.minutes().count());
        int s = static_cast<int>(hms.seconds().count());
        int msec = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(hms.subseconds()).count());

        char buf[16];   /* "HH:MM:SS.mmm" = 12 + NUL */
        int n = std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", h, m, s, msec);
        if (n > 0)
            sink.put(std::string_view(buf, static_cast<std::size_t>(n)));
    }

    void
    put_hms_usec(PpSink & sink, xo::time::nanos dt)
    {
        auto hms = std::chrono::hh_mm_ss(dt);
        int h = static_cast<int>(hms.hours().count());
        int m = static_cast<int>(hms.minutes().count());
        int s = static_cast<int>(hms.seconds().count());
        int usec = static_cast<int>(
            std::chrono::duration_cast<std::chrono::microseconds>(hms.subseconds()).count());

        char buf[20];   /* "HH:MM:SS.uuuuuu" = 15 + NUL */
        int n = std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", h, m, s, usec);
        if (n > 0)
            sink.put(std::string_view(buf, static_cast<std::size_t>(n)));
    }
} /*namespace xo::pp*/

/* end pp_time.cpp */
