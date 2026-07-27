/** @file pp_time.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  PpSink-native time-of-day formatting.
 *
 *  These write a fixed-width time-of-day string directly to a PpSink as a
 *  single put() token, avoiding the operator<< fallback path that the
 *  ostream-based xo/timeutil/timeutil_iostream.hpp printers go through
 *  (stream_open + ostream + streambuf).  Each writes a constant number of
 *  characters, so a small stack buffer suffices.
 *
 *  Lives in xo-ppsink (not xo-timeutil): timeutil sits below ppsink in the
 *  level order and can't know about PpSink.
 **/

#pragma once

#include "PpSink.hpp"
#include <xo/timeutil/timeutil.hpp>   /* xo::time::nanos */

namespace xo::pp {
    /** write "HH:MM:SS.mmm" (12 chars) for @p dt (duration since midnight) **/
    void put_hms_msec(PpSink & sink, xo::time::nanos dt);

    /** write "HH:MM:SS.uuuuuu" (15 chars) for @p dt (duration since midnight) **/
    void put_hms_usec(PpSink & sink, xo::time::nanos dt);

    /** write "HH:MM:SS.mmm" for the UTC time-of-day of @p t0 **/
    void put_utc_hms_msec(PpSink & sink, xo::time::utc_nanos t0);

    /** write "yyyymmdd:HH:MM:SS.uuuuuu" (UTC) for @p t0 **/
    void put_utc_ymd_hms_usec(PpSink & sink, xo::time::utc_nanos t0);

    /** write ISO-8601 "yyyy-mm-ddThh:mm:ss.mmmZ" (UTC) for @p t0 **/
    void put_iso8601(PpSink & sink, xo::time::utc_nanos t0);
} /*namespace xo::pp*/

/* end pp_time.hpp */
