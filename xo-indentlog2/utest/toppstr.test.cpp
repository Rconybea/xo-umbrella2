/* @file toppstr.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * xo::pp::toppstr -- render through a PrettySink, i.e. WITH line breaking.
 * Counterpart to xo::pp::tostr, which uses a FlatSink and never breaks.
 *
 * Expectations here were read off observed output, not predicted.
 */

#include "print/toppstr.hpp"
#include <xo/ppsink/pretty_array.hpp>
#include <xo/ppsink/tostr.hpp>
#include <catch2/catch.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    namespace ut {
        using xo::pp::PpConfig;

        namespace {
            /** an array is enough structure to break: Prettifier<std::array>
             *  puts a split() between elements.
             **/
            std::array<int, 8> s_value = { 111, 222, 333, 444, 555, 666, 777, 888 };

            std::string
            render_at(std::uint32_t margin) {
                return xo::pp::toppstr(PpConfig().with_soft_right_margin(margin),
                                       s_value);
            }

            struct Testcase_Toppstr {
                Testcase_Toppstr(std::uint32_t margin, bool expect_break)
                    : margin_{margin}, expect_break_{expect_break} {}

                /** soft right margin to render at **/
                std::uint32_t margin_;
                /** whether the rendering is expected to span >1 line **/
                bool expect_break_;
            };

            std::vector<Testcase_Toppstr>
            s_testcase_v = {
                Testcase_Toppstr(200, false),
                Testcase_Toppstr(100, false),
                Testcase_Toppstr(20,  true),
                Testcase_Toppstr(8,   true),
            };
        } /*namespace*/

        TEST_CASE("toppstr-breaks-at-margin", "[toppstr]") {
            /* the whole point of toppstr: unlike tostr (FlatSink), the margin
             * governs whether the rendering breaks.
             */
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_Toppstr & tc = s_testcase_v[i_tc];

                std::string s = render_at(tc.margin_);

                INFO("i_tc=" << i_tc << " margin=" << tc.margin_ << " rendered=[" << s << "]");

                REQUIRE((s.find('\n') != std::string::npos) == tc.expect_break_);

                /* breaking must not lose content */
                for (int elt : s_value) {
                    INFO("element " << elt);
                    REQUIRE(s.find(std::to_string(elt)) != std::string::npos);
                }
            }
        }

        TEST_CASE("toppstr-wide-margin-matches-tostr", "[toppstr]") {
            /* at a margin wide enough that nothing breaks, toppstr and tostr
             * must agree -- they differ only in whether breaking is possible.
             */
            REQUIRE(render_at(200) == xo::pp::tostr(s_value));
        }

        TEST_CASE("toppstr-is-repeatable", "[toppstr]") {
            /* The hazard toppstr exists to make unreachable: two PrettySinks
             * sharing an ArenaConfig name interfere, and the symptom is WRONG
             * INDENTATION in whichever renders second -- a silent wrong answer,
             * not an error.  Six files carried a comment warning about it.
             * Rendering the same input repeatedly must give the same result.
             */
            constexpr int c_n = 8;

            std::string first = render_at(20);

            for (int i = 0; i < c_n; ++i) {
                INFO("iteration " << i << " first=[" << first << "]");
                REQUIRE(render_at(20) == first);
            }
        }

        TEST_CASE("toppstr-overloads", "[toppstr]") {
            /* the no-config overload must not be shadowed by the
             * PpConfig-taking one, nor vice versa
             */
            REQUIRE(xo::pp::toppstr(123) == "123");
            REQUIRE(xo::pp::toppstr(PpConfig(), 123) == "123");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end toppstr.test.cpp */
