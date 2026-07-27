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
    using xo::time::nanos;
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
} /*namespace ut*/

/* end pp_time.test.cpp */
