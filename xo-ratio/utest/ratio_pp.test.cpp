/** @file ratio_pp.test.cpp
 *
 *  exercise Prettifier<ratio<Int>> from xo/ratio/ratio_pp.hpp.
 *
 *  The load-bearing assertion here is the LAST one: that the ppsink rendering
 *  and the ostream rendering agree byte for byte.  They are two separate
 *  pieces of code (std::to_chars + put() vs operator<<), and if they drift, a
 *  ratio starts printing differently depending on whether a TU happens to
 *  include ratio_pp.hpp.
 **/

#include "xo/ratio/ratio_pp.hpp"
#include "xo/ratio/ratio_iostream.hpp"
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/tag.hpp>
#include <xo/ppsink/tag_ostream.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>

namespace xo {
    namespace ut {
        using xo::pp::FlatSink;
        using xo::ratio::ratio;
        using std::stringstream;

        namespace {
            /** render @p x through the ppsink path **/
            template <typename T>
            std::string
            pp_str(const T & x) {
                stringstream ss;
                FlatSink sink(ss);
                sink.pp(x);
                return ss.str();
            }

            /** render @p x through the ostream path **/
            template <typename T>
            std::string
            os_str(const T & x) {
                stringstream ss;
                ss << x;
                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("ratio_pp-basic", "[ratio][ratio_pp]") {
            REQUIRE(pp_str(ratio<int>(2, 3)) == "<ratio 2/3>");
        }

        TEST_CASE("ratio_pp-negative", "[ratio][ratio_pp]") {
            REQUIRE(pp_str(ratio<int>(-2, 3)) == "<ratio -2/3>");
        }

        TEST_CASE("ratio_pp-integer-valued", "[ratio][ratio_pp]") {
            /* an integer-valued ratio still shows its denominator, matching
             * operator<<.  (ratio::to_str() is the terse form; that is a
             * different rendering, and not what this Prettifier reproduces.)
             */
            REQUIRE(pp_str(ratio<int>(4, 1)) == "<ratio 4/1>");
        }

        TEST_CASE("ratio_pp-extreme-values-fit-the-buffer", "[ratio][ratio_pp]") {
            /* the widest renderings the buffer must hold: to_chars would
             * silently produce a short write if it were undersized.
             */
            constexpr auto i32_min = std::numeric_limits<std::int32_t>::min();
            constexpr auto i64_min = std::numeric_limits<std::int64_t>::min();

            REQUIRE(pp_str(ratio<std::int32_t>(i32_min, 1))
                    == "<ratio -2147483648/1>");
            REQUIRE(pp_str(ratio<std::int64_t>(i64_min, 1))
                    == "<ratio -9223372036854775808/1>");
        }

        TEST_CASE("ratio_pp-usable-as-a-tag-value", "[ratio][ratio_pp]") {
            REQUIRE(pp_str(xo::pp::tag("r", ratio<int>(2, 3))) == ":r <ratio 2/3>");
        }

        TEST_CASE("ratio_pp-agrees-with-ostream", "[ratio][ratio_pp]") {
            /* the anti-divergence test.  Fails the moment Prettifier and
             * operator<< disagree -- which is what would make a ratio's
             * rendering depend on which headers a TU included.
             */
            REQUIRE(pp_str(ratio<int>(2, 3)) == os_str(ratio<int>(2, 3)));
            REQUIRE(pp_str(ratio<int>(-7, 6)) == os_str(ratio<int>(-7, 6)));
            REQUIRE(pp_str(ratio<int>(0, 1)) == os_str(ratio<int>(0, 1)));
            REQUIRE(pp_str(ratio<std::int64_t>(123456789, 987654321))
                    == os_str(ratio<std::int64_t>(123456789, 987654321)));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end ratio_pp.test.cpp */
