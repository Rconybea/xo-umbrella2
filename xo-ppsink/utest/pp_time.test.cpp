/** @file pp_time.test.cpp **/

#include <xo/ppsink/pp_time.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <chrono>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::put_hms_msec;
    using xo::pp::put_hms_usec;
    using xo::pp::put_utc_hms_msec;
    using xo::pp::put_utc_ymd_hms_usec;
    using xo::pp::put_iso8601;
    using xo::time::nanos;
    using xo::time::utc_nanos;
    using xo::time::timeutil;
    using std::stringstream;

    static std::string msec_of(nanos dt) {
        stringstream ss; FlatSink s(ss); put_hms_msec(s, dt); return ss.str();
    }
    static std::string usec_of(nanos dt) {
        stringstream ss; FlatSink s(ss); put_hms_usec(s, dt); return ss.str();
    }

    TEST_CASE("pp_time.hms", "[pp_time]") {
        using namespace std::chrono;

        nanos dt = hours(1) + minutes(2) + seconds(3) + milliseconds(456) + microseconds(789);

        REQUIRE(msec_of(dt) == "01:02:03.456");
        REQUIRE(usec_of(dt) == "01:02:03.456789");

        REQUIRE(msec_of(nanos(0)) == "00:00:00.000");
        REQUIRE(usec_of(hours(23) + minutes(59) + seconds(59)) == "23:59:59.000000");
    }

    TEST_CASE("pp_time.utc", "[pp_time]") {
        /* a fixed UTC instant: 2023-09-21 16:29:35.123456 UTC */
        utc_nanos t = timeutil::ymd_hms_usec(20230921, 162935, 123456);

        auto render = [](auto fn, utc_nanos tt) {
            stringstream ss; FlatSink s(ss); fn(s, tt); return ss.str();
        };

        REQUIRE(render(put_utc_hms_msec, t)     == "16:29:35.123");
        REQUIRE(render(put_utc_ymd_hms_usec, t) == "20230921:16:29:35.123456");
        REQUIRE(render(put_iso8601, t)          == "2023-09-21T16:29:35.123Z");
    }
} /*namespace ut*/

/* end pp_time.test.cpp */
