/* @file fixed.test.cpp */

#include "xo/indentlog/print/quoted.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace xo;
using namespace xo::print;

namespace ut {
    struct quoted_tcase {
        quoted_tcase() = default;
        quoted_tcase(std::string x, bool unq_flag, std::string s)
            : x_{x}, unq_flag_{unq_flag}, s_{std::move(s)} {}

        /* string to be printed-in-machine-readable-form */
        std::string x_;
        /* if true:  omit surrounding " chars when unambiguous
         *           (printed string does not contain spaces or escaped chars)
         * if false: always require surrounding " chars
         */
        bool unq_flag_ = true;
        /* expected result */
        std::string s_;
    }; /*quoted_tcase*/

    std::vector<quoted_tcase> s_quoted_tcase_v(
        {
            quoted_tcase("", true, "\"\""),
            quoted_tcase("", false, "\"\""),

            quoted_tcase("foo", true, "foo"),
            quoted_tcase("foo", false, "\"foo\""),

            quoted_tcase("foo\n", true, "\"foo\\\n\""),
            quoted_tcase("foo\n", false, "\"foo\\\n\""),

            quoted_tcase("two words", true, "\"two words\""),
            quoted_tcase("two words", false, "\"two words\""),

            quoted_tcase("1st\n2nd", true, "\"1st\\\n2nd\""),
            quoted_tcase("1st\n2nd", true, "\"1st\\\n2nd\""),

            quoted_tcase("misakte\rfix", true, "\"misakte\\\rfix\""),
            quoted_tcase("misakte\rfix", true, "\"misakte\\\rfix\""),

            quoted_tcase("\"oh!\", she said", true, "\"\\\"oh!\\\", she said\""),
            quoted_tcase("\"oh!\", she said", false, "\"\\\"oh!\\\", she said\""),

            quoted_tcase("<object printer output>", true, "<object printer output>"),
            quoted_tcase("<object printer output>", false, "<object printer output>"),
        });

    TEST_CASE("quoted", "[quoted]") {
        for (std::uint32_t i_tc = 0, z_tc = s_quoted_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            quoted_tcase const & tc = s_quoted_tcase_v[i_tc];

            INFO(tostr(xtag("i_tc", i_tc), xtag("x", tc.x_), xtag("unq_flag", tc.unq_flag_)));

            std::stringstream ss;
            if (tc.unq_flag_)
                ss << unq(tc.x_);
            else
                ss << quoted(tc.x_);

            INFO(xtag("ss.str", ss.str()));

            REQUIRE(ss.str() == tc.s_);
        }

        REQUIRE(s_quoted_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end quoted.test.cpp */
