/** @file pp_time.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include <xo/ppsink/pp_time.hpp>

namespace xo::pp {
    void
    put_hms_msec(PpSink & sink, xo::time::nanos dt)
    {
        char buf[16];   /* "HH:MM:SS.mmm" = 12 + NUL */
        sink.put(xo::time::format_hms_msec(buf, sizeof(buf), dt));
    }

    void
    put_hms_usec(PpSink & sink, xo::time::nanos dt)
    {
        char buf[20];   /* "HH:MM:SS.uuuuuu" = 15 + NUL */
        sink.put(xo::time::format_hms_usec(buf, sizeof(buf), dt));
    }

    void
    put_utc_hms_msec(PpSink & sink, xo::time::utc_nanos t0)
    {
        char buf[16];
        sink.put(xo::time::format_utc_hms_msec(buf, sizeof(buf), t0));
    }

    void
    put_utc_ymd_hms_usec(PpSink & sink, xo::time::utc_nanos t0)
    {
        char buf[32];   /* "yyyymmdd:HH:MM:SS.uuuuuu" = 23 + NUL */
        sink.put(xo::time::format_utc_ymd_hms_usec(buf, sizeof(buf), t0));
    }

    void
    put_iso8601(PpSink & sink, xo::time::utc_nanos t0)
    {
        char buf[32];   /* "yyyy-mm-ddThh:mm:ss.mmmZ" = 24 + NUL */
        sink.put(xo::time::format_iso8601(buf, sizeof(buf), t0));
    }
} /*namespace xo::pp*/

/* end pp_time.cpp */
