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
#include "Prettifier.hpp"
#include <xo/timeutil/timeutil.hpp> /* xo::time::nanos */

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

    // ----------------------------------------------------------------
    // value wrappers
    //
    // The put_*() functions above write to a sink.  These wrap a time in a
    // value you can hand to anything that prints -- sink.pp(x),
    // xtag("k", x), tostr(x) -- which is the shape call sites actually want,
    // and the shape the legacy xo-indentlog print/time.hpp had.
    //
    // Every format here is deliberately SPACE-FREE, so a rendered datetime is
    // a single token and reads back as a literal.  That is the reason not to
    // fall back on C++20 <chrono>'s inserter, whose default
    // ("2022-09-26 09:30:00.123456000") embeds a space.
    // ----------------------------------------------------------------

    /** @brief a time to print ISO-8601: @c 2022-09-26T09:30:00.123Z **/
    class iso8601 {
    public:
        explicit constexpr iso8601(xo::time::utc_nanos t0) noexcept : t0_{t0} {}

        constexpr xo::time::utc_nanos t0() const noexcept { return t0_; }

    private:
        xo::time::utc_nanos t0_;
    };

    template <>
    struct Prettifier<iso8601> {
        static void print(PpSink & sink, iso8601 x) { put_iso8601(sink, x.t0()); }
    };

    /** @brief a time-of-day to print to milliseconds: @c 09:30:00.123
     *
     *  Construct from a duration-since-midnight, or use @ref utc / @ref local
     *  to take the time-of-day part of an absolute time.
     **/
    class hms_msec {
    public:
        explicit constexpr hms_msec(xo::time::nanos dt) noexcept : dt_{dt} {}

        /** time-of-day of @p t0 in UTC **/
        static hms_msec utc(xo::time::utc_nanos t0) {
            return hms_msec(xo::time::timeutil::utc_split_vs_midnight(t0).second);
        }
        /** time-of-day of @p t0 in local time **/
        static hms_msec local(xo::time::utc_nanos t0) {
            return hms_msec(xo::time::timeutil::local_split_vs_midnight(t0).second);
        }

        constexpr xo::time::nanos dt() const noexcept { return dt_; }

    private:
        xo::time::nanos dt_;
    };

    template <>
    struct Prettifier<hms_msec> {
        static void print(PpSink & sink, hms_msec x) { put_hms_msec(sink, x.dt()); }
    };

    /** @brief a time-of-day to print to microseconds: @c 09:30:00.123456 **/
    class hms_usec {
    public:
        explicit constexpr hms_usec(xo::time::nanos dt) noexcept : dt_{dt} {}

        /** time-of-day of @p t0 in UTC **/
        static hms_usec utc(xo::time::utc_nanos t0) {
            return hms_usec(xo::time::timeutil::utc_split_vs_midnight(t0).second);
        }
        /** time-of-day of @p t0 in local time **/
        static hms_usec local(xo::time::utc_nanos t0) {
            return hms_usec(xo::time::timeutil::local_split_vs_midnight(t0).second);
        }

        constexpr xo::time::nanos dt() const noexcept { return dt_; }

    private:
        xo::time::nanos dt_;
    };

    template <>
    struct Prettifier<hms_usec> {
        static void print(PpSink & sink, hms_usec x) { put_hms_usec(sink, x.dt()); }
    };

    /** @brief a time to print date+time: @c 20220926:09:30:00.123456 (UTC) **/
    class ymd_hms_usec {
    public:
        explicit constexpr ymd_hms_usec(xo::time::utc_nanos t0) noexcept : t0_{t0} {}

        constexpr xo::time::utc_nanos t0() const noexcept { return t0_; }

    private:
        xo::time::utc_nanos t0_;
    };

    template <>
    struct Prettifier<ymd_hms_usec> {
        static void print(PpSink & sink, ymd_hms_usec x) {
            put_utc_ymd_hms_usec(sink, x.t0());
        }
    };

    // ----------------------------------------------------------------
    // bare chrono types
    //
    // These give an unwrapped utc_nanos / nanos the same rendering the legacy
    // xo-indentlog print/time.hpp gave it -- but legitimately.  Legacy did it
    // by declaring operator<< inside `namespace std::chrono`, which is
    // undefined behaviour (only explicit specializations of std templates may
    // be added there).  A Prettifier<> specialization lives in xo::pp and is
    // found by the Prettifier machinery, not by ADL, so no such trick is
    // needed.
    //
    // Consequence worth knowing: this covers every ppsink path -- sink.pp(t),
    // xtag("k", t), tostr(t), pp_to_stream(os, t).  It does NOT cover a raw
    // `os << t`, which resolves by ADL into std::chrono and so gets C++20's
    // space-bearing default.  Use pp_to_stream(), or wrap explicitly.
    // ----------------------------------------------------------------

    template <>
    struct Prettifier<xo::time::utc_nanos> {
        static void print(PpSink & sink, xo::time::utc_nanos t0) {
            put_utc_ymd_hms_usec(sink, t0);
        }
    };

    template <>
    struct Prettifier<xo::time::nanos> {
        static void print(PpSink & sink, xo::time::nanos dt) {
            put_hms_usec(sink, dt);
        }
    };
} /*namespace xo::pp*/

/* end pp_time.hpp */
