/* @file fixed.test.cpp */

#include "xo/indentlog/print/quoted.hpp"
//#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/hex.hpp"
#include <catch2/catch.hpp>
#include <sstream>

namespace ut {
    using namespace xo;
    using namespace xo::print;

    struct quoted_tcase {
        quoted_tcase() = default;
        quoted_tcase(std::string x, bool unq_flag, std::string s)
            : x_{std::move(x)}, unq_flag_{unq_flag}, s_{std::move(s)} {}

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

            quoted_tcase("foo\n", true, "\"foo\\n\""),
#if __GNUC__ >= 13 && __GNUC_MINOR__ >= 2
            /* writes "foo\n", but gets turned into newline somewhere.  only on very recent gcc. (not on 11.4.0) */
            quoted_tcase("foo\n", false, "\"foo\n\""),
#else
            quoted_tcase("foo\n", false, "\"foo\\n\""),
#endif

            quoted_tcase("two words", true, "\"two words\""),
            quoted_tcase("two words", false, "\"two words\""),

            quoted_tcase("1st\n2nd", true, "\"1st\\n2nd\""),
#if __GNUC__ >= 13 && __GNUC_MINOR__ >= 2
            /* writes "1st\\nsecond", but still gets turned into newline somewhere.  only on very recent gcc. (not on 11.4.0) */
            quoted_tcase("1st\n2nd", false, "\"1st\n2nd\""),
#else
            quoted_tcase("1st\n2nd", false, "\"1st\\n2nd\""),
#endif

            quoted_tcase("misakte\rfix", true, "\"misakte\\rfix\""),
#if __GNUC__ >= 13 && __GNUC_MINOR__ >= 2
            quoted_tcase("misakte\rfix", false, "\"misakte\rfix\""),
#else
            quoted_tcase("misakte\rfix", false, "\"misakte\\rfix\""),
#endif

            quoted_tcase("\"oh!\", she said", true, "\"\\\"oh!\\\", she said\""),
            quoted_tcase("\"oh!\", she said", false, "\"\\\"oh!\\\", she said\""),

            quoted_tcase("<object printer output>", true, "<object printer output>"),
#if __GNUC__ >= 13 && __GNUC_MINOR__ >= 2
            quoted_tcase("<object printer output>", false, "\"<object printer output>\""),
#else
            quoted_tcase("<object printer output>", false, "\"<object printer output>\""),
#endif
        });

    TEST_CASE("quoted", "[quoted]") {
        for (std::uint32_t i_tc = 0, z_tc = s_quoted_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            quoted_tcase const & tc = s_quoted_tcase_v[i_tc];

            /* NOTE: don't use tag()/xtag() here,
             *       since implementation relies on the inserter we are testing
             */

            INFO(tostr("i_tc=", i_tc, " unq_flag=", tc.unq_flag_));
            INFO("tc.x_ ----------------");
            INFO(tostr("[", tc.x_, "]"));
            INFO("tc.x_ ----------------");

            bool special_char = (tc.x_.find_first_of(" \"\n\r\\") != std::string::npos);

            INFO(tostr("special_char=", special_char));

            std::stringstream ss;
            if (tc.unq_flag_)
                ss << unq(tc.x_);
            else
                ss << quoted(tc.x_);

            INFO("tc.s ----------------");
            INFO(tostr("[", tc.s_, "]"));
            INFO("tc.s ----------------");
            INFO("ss.str ----------------");
            INFO(tostr("[", hex_view(ss.view().begin(), ss.view().end(), true), "]"));
            INFO(tostr("[", ss.str(), "]"));
            INFO("ss.str ----------------");

            REQUIRE(ss.str() == tc.s_);

            if (ss.str() != tc.s_)
                break;
        }

        REQUIRE(s_quoted_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end quoted.test.cpp */
