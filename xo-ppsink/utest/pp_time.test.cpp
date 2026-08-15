/** @file pp_time.test.cpp **/

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pp_time.hpp>
#include <xo/ppsink/pp_time_ostream.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tostr.hpp>
#include <catch2/catch.hpp>
#include <chrono>
#include <sstream>

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
        stringstream ss; FlatSink s(ss.rdbuf()); put_hms_msec(s, dt); return ss.str();
    }
    static std::string usec_of(nanos dt) {
        stringstream ss; FlatSink s(ss.rdbuf()); put_hms_usec(s, dt); return ss.str();
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
            stringstream ss; FlatSink s(ss.rdbuf()); fn(s, tt); return ss.str();
        };

        REQUIRE(render(put_utc_hms_msec, t)     == "16:29:35.123");
        REQUIRE(render(put_utc_ymd_hms_usec, t) == "20230921:16:29:35.123456");
        REQUIRE(render(put_iso8601, t)          == "2023-09-21T16:29:35.123Z");
    }

    // ----------------------------------------------------------------
    // value wrappers + Prettifiers (parity with legacy xo-indentlog
    // print/time.hpp, which is being retired)
    // ----------------------------------------------------------------

    namespace {
        /** render @p x through the ppsink path (Prettifier) **/
        template <typename T>
        std::string pp_of(const T & x) {
            stringstream ss; FlatSink s(ss.rdbuf()); s.pp(x); return ss.str();
        }
        /** render @p x through operator<< (pp_time_ostream.hpp) **/
        template <typename T>
        std::string os_of(const T & x) {
            stringstream ss; ss << x; return ss.str();
        }
        /** a fixed instant: 2022-09-26 09:30:00.123456 UTC **/
        utc_nanos sample() {
            return timeutil::ymd_hms_usec(20220926, 93000, 123456);
        }
    } /*namespace*/

    TEST_CASE("pp_time.iso8601-wrapper", "[pp_time][wrapper]") {
        xo::pp::iso8601 x(sample());

        REQUIRE(pp_of(x) == "2022-09-26T09:30:00.123Z");
        REQUIRE(os_of(x) == pp_of(x));
    }

    TEST_CASE("pp_time.hms-wrappers", "[pp_time][wrapper]") {
        utc_nanos t0 = sample();

        REQUIRE(pp_of(xo::pp::hms_msec::utc(t0)) == "09:30:00.123");
        REQUIRE(pp_of(xo::pp::hms_usec::utc(t0)) == "09:30:00.123456");

        /* the ostream bridge must agree byte-for-byte with the sink path */
        REQUIRE(os_of(xo::pp::hms_msec::utc(t0)) == pp_of(xo::pp::hms_msec::utc(t0)));
        REQUIRE(os_of(xo::pp::hms_usec::utc(t0)) == pp_of(xo::pp::hms_usec::utc(t0)));

        /* local() must at least produce a well-formed time-of-day; the value
         * depends on the host timezone, so don't pin the digits
         */
        std::string loc = pp_of(xo::pp::hms_usec::local(t0));
        REQUIRE(loc.size() == std::string("09:30:00.123456").size());
        REQUIRE(loc.find(':') != std::string::npos);
    }

    TEST_CASE("pp_time.ymd_hms_usec-wrapper", "[pp_time][wrapper]") {
        xo::pp::ymd_hms_usec x(sample());

        REQUIRE(pp_of(x) == "20220926:09:30:00.123456");
        REQUIRE(os_of(x) == pp_of(x));
    }

    TEST_CASE("pp_time.bare-chrono-types", "[pp_time][wrapper]") {
        /* Prettifier<utc_nanos> / Prettifier<nanos> reproduce what legacy
         * xo-indentlog gave a bare time_point -- but without declaring an
         * operator<< inside namespace std::chrono (which was UB).
         */
        utc_nanos t0 = sample();
        nanos dt = timeutil::utc_split_vs_midnight(t0).second;

        REQUIRE(pp_of(t0) == "20220926:09:30:00.123456");
        REQUIRE(pp_of(dt) == "09:30:00.123456");

        /* reached through the ordinary printing vocabulary, not just sink.pp */
        REQUIRE(xo::pp::tostr(t0) == "20220926:09:30:00.123456");
        REQUIRE(pp_of(xo::pp::tag("t", t0)) == ":t 20220926:09:30:00.123456");
    }

    TEST_CASE("pp_time.formats-are-space-free", "[pp_time][wrapper]") {
        /* THE invariant: a rendered datetime must be a single token, so it
         * reads back as a literal.  This is why xo does not fall back on
         * C++20 <chrono>'s inserter, whose default embeds a space
         * ("2022-09-26 09:30:00.123456000").
         */
        utc_nanos t0 = sample();
        nanos dt = timeutil::utc_split_vs_midnight(t0).second;

        const std::string rendered[] = {
            pp_of(xo::pp::iso8601(t0)),
            pp_of(xo::pp::hms_msec::utc(t0)),
            pp_of(xo::pp::hms_usec::utc(t0)),
            pp_of(xo::pp::hms_msec::local(t0)),
            pp_of(xo::pp::hms_usec::local(t0)),
            pp_of(xo::pp::ymd_hms_usec(t0)),
            pp_of(t0),
            pp_of(dt),
        };

        for (const auto & s : rendered) {
            INFO(s);
            REQUIRE(!s.empty());
            REQUIRE(s.find(' ') == std::string::npos);
            REQUIRE(s.find('\t') == std::string::npos);
        }
    }
} /*namespace ut*/

/* end pp_time.test.cpp */
