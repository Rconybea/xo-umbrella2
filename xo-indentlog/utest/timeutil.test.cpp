/* @file timeutil.test.cpp */

#include "xo/indentlog/timeutil/timeutil.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;
using namespace xo::time;
using namespace std::chrono;

namespace ut {
    template <typename FromTime>
    inline utc_micros to_micros(FromTime tm) {
        return std::chrono::time_point_cast<xo::time::microseconds>(tm);
    } /*to_micros*/

    TEST_CASE("epoch", "[timeutil]") {
        //tag_config::tag_color = color_spec_type::none();
        using xo::time::microseconds;


        utc_nanos t0 = timeutil::epoch();

        REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(0));
    } /*TEST_CASE(epoch)*/

    TEST_CASE("ymd_hms", "[timeutil]") {
        {
            utc_nanos t0 = timeutil::ymd_hms(19700101 /*ymd*/, 0 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(0));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700101 /*ymd*/, 1 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(1));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700101 /*ymd*/, 100 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(60));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700101 /*ymd*/, 10000 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(3600));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700101 /*ymd*/, 235959 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(86399));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700102 /*ymd*/, 235959 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(86400 + 86399));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700131 /*ymd*/, 235959 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(30 * 86400 + 86399));
        }

        {
            utc_nanos t0 = timeutil::ymd_hms(19700201 /*ymd*/, 235959 /*hms*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(31 * 86400 + 86399));
        }
    } /*TEST_CASE(ymd_hms)*/

    TEST_CASE("ymd_midnight", "[timeutil]") {
        {
            utc_nanos t0 = timeutil::ymd_midnight(19700101 /*ymd*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(0));
        }

        {
            utc_nanos t0 = timeutil::ymd_midnight(19700102 /*ymd*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(86400));
        }

        {
            utc_nanos t0 = timeutil::ymd_midnight(19700131 /*ymd*/);
            REQUIRE(std::chrono::system_clock::to_time_t(to_micros(t0)) == std::time_t(30 * 86400));
        }

        {
            utc_nanos t0 = timeutil::ymd_midnight(19700201 /*ymd*/);
            REQUIRE(system_clock::to_time_t(to_micros(t0)) == std::time_t(31 * 86400));
        }
    } /*TEST_CASE(ymd_midnight)*/

    struct timeutil_tcase {
        timeutil_tcase() = default;
        timeutil_tcase(uint32_t ymd, uint32_t hms, uint32_t usec,
                       std::time_t epoch_sec, std::time_t midnight_sec, std::uint32_t fractional_sec, std::uint32_t fractional_usec,
                       std::string const & utc_ymd_hms_usec_str,
                       std::string const & iso8601_str)
            : ymd_{ymd}, hms_{hms}, usec_{usec},
              epoch_sec_{epoch_sec},
              midnight_sec_{midnight_sec},
              fractional_sec_{fractional_sec},
              fractional_usec_{fractional_usec},
              utc_ymd_hms_usec_str_{utc_ymd_hms_usec_str},
              iso8601_str_{iso8601_str} {}

        /* target time value to test */
        std::uint32_t ymd_ = 19700101;
        std::uint32_t hms_ = 0;
        std::uint32_t usec_ = 0;

        std::time_t epoch_sec_ = 0;
        std::time_t midnight_sec_ = 0;
        std::uint32_t fractional_sec_ = 0;
        std::uint32_t fractional_usec_ = 0;

        std::string utc_ymd_hms_usec_str_;
        std::string iso8601_str_;
    }; /*timeutil_tcase*/

    std::vector<timeutil_tcase> s_timeutil_tcase_v(
        /*              -------- inputs -------  ------------------------------------------------ outputs ---------------------------------------
         *                                                                fractional_usec
         *                                                         fractional_sec       |
         *                   ymd     hms    usec  epoch_sec  midnight_sec       v       v        utc_ymd_hms_usec_str                 iso8601_str
         */
        {
         timeutil_tcase(19700101,      0,      0,         0,            0,      0,      0, "19700101:00:00:00.000000", "1970-01-01T00:00:00.000Z"),
         timeutil_tcase(19700101,      0,      1,         0,            0,      0,      1, "19700101:00:00:00.000001", "1970-01-01T00:00:00.000Z"),
         timeutil_tcase(19700101,      0, 123456,         0,            0,      0, 123456, "19700101:00:00:00.123456", "1970-01-01T00:00:00.123Z"),
         timeutil_tcase(19700101,      0, 500000,         0,            0,      0, 500000, "19700101:00:00:00.500000", "1970-01-01T00:00:00.500Z"),
         timeutil_tcase(19700101,      0, 987654,         0,            0,      0, 987654, "19700101:00:00:00.987654", "1970-01-01T00:00:00.987Z"),

         timeutil_tcase(19700101,      0, 999999,         0,            0,      0, 999999, "19700101:00:00:00.999999", "1970-01-01T00:00:00.999Z"),

         timeutil_tcase(19700101,      1, 999999,         1,            0,      1, 999999, "19700101:00:00:01.999999", "1970-01-01T00:00:01.999Z"),

         timeutil_tcase(19700101,    100, 999999,        60,            0,     60, 999999, "19700101:00:01:00.999999", "1970-01-01T00:01:00.999Z"),
         timeutil_tcase(19700101,  10000, 999999,      3600,            0,   3600, 999999, "19700101:01:00:00.999999", "1970-01-01T01:00:00.999Z"),
         timeutil_tcase(19700101, 235959, 999999, 24*3600-1,            0,  86399, 999999, "19700101:23:59:59.999999", "1970-01-01T23:59:59.999Z"),

         timeutil_tcase(19700102,    100, 999999,  86400+60,        86400,     60, 999999, "19700102:00:01:00.999999", "1970-01-02T00:01:00.999Z"),

        });

    TEST_CASE("ymd_hms_usec", "[timeutil]") {
        for (std::uint32_t i_tc = 0, z_tc = s_timeutil_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            timeutil_tcase const & tc = s_timeutil_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("ymd", tc.ymd_)));
            INFO(xtag("tc.epoch_sec", tc.epoch_sec_));
            INFO(xtag("tc.utc_ymd_hms_usec_str", tc.utc_ymd_hms_usec_str_));

            utc_nanos const t0 = timeutil::ymd_hms_usec(tc.ymd_, tc.hms_, tc.usec_);
            REQUIRE(system_clock::to_time_t(to_micros(t0)) == std::time_t(tc.epoch_sec_));

            auto x = timeutil::utc_split_vs_midnight(t0);
            REQUIRE(system_clock::to_time_t(to_micros(x.first)) == tc.midnight_sec_);
            REQUIRE(x.second == seconds(tc.fractional_sec_) + microseconds(tc.fractional_usec_));

            {
                std::stringstream ss;
                timeutil::print_utc_ymd_hms_usec(t0, ss);
                REQUIRE(ss.str() == tc.utc_ymd_hms_usec_str_);
            }

            {
                std::stringstream ss;
                timeutil::print_iso8601(t0, ss);
                REQUIRE(ss.str() == tc.iso8601_str_);
            }
        }
    } /*TEST_CASE(ymd_hms_usec)*/
} /*namespace ut*/

/* end timeutil.test.cpp */
