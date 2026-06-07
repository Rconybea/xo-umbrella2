/* @file fixed.test.cpp */

#include "xo/indentlog/scope.hpp"
#include "xo/indentlog/print/quoted.hpp"
//#include "xo/indentlog/print/tag.hpp"
#include "xo/indentlog/print/hex.hpp"
#include <catch2/catch.hpp>
#include <sstream>

namespace ut {
    using namespace xo;
    using namespace xo::print;

    struct quot_tcase {
        quot_tcase() = default;
        quot_tcase(std::string x, bool unq_flag, std::string s)
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
    }; /*quot_tcase*/

    /* NOTE: spelled out tests here in aftermath
     *       of hard-to-diagnose regression in gcc 13.2;
     *       turned out to have something to originate in confusion
     *       between xo::print::quoted and std::quoted.
     *
     *       Problem does not occur in gcc 12.3 and earlier,
     *       perhaps some alias for std::quoted appears somewhere in global
     *       namespace??
     *
     *       Resolved by renaming xo::print::quoted -> xo::print::quot
     */

    TEST_CASE("sstream.1char", "[sstream]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.1char"));

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << '\\';

            std::string str = ss.str();

            log && log("after: lone escaped backslash");
            log && log(hex_view(str.data(), str.data() + str.size(), true));

            //REQUIRE(ss.view() == std::string_view("\\"));  // n/avail on osx
            REQUIRE(str == std::string("\\"));

            ss << 'n';

            std::string str2 = ss.str();

            log && log("after: lone 'n' char");
            log && log(hex_view(str2.data(), str2.data() + str2.size(), true));

            // REQUIRE(ss.view() == std::string_view("\\n"));  // n/avail on osx
            REQUIRE(str2 == std::string("\\n"));

            log && log("ss.str()=[", str2, "]");
        }
    } /*TEST_CASE(sstream.1char)*/

    TEST_CASE("sstream.2bslash", "[sstream]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.2bslash"));

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << "\\\\";

            std::string str = ss.str();

            log && log("after: 2x escaped backslash");
            log && log(hex_view(str.data(), str.data() + str.size(), true));

            REQUIRE(str == std::string("\\\\"));

            log && log("ss.str()=[", ss.str(), "]");
        }
    } /*TEST_CASE(sstream.2bslash)*/

    TEST_CASE("sstream.2char", "[sstream]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.2char"));

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << "\\n";

            std::string str = ss.str();

            log && log("after: '\\n' escaped backslash + n");
            log && log(hex_view(str.data(), str.data() + str.size(), true));
        }
    }

    TEST_CASE("sstream.3char", "[sstream]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.3char"));

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            /* this is what quot("\\n") should wind up executing.. */
            ss << "\\\\";
            ss << 'n';

            std::string str = ss.str();

            log && log("after: '\\\\n' 2x escaped backslash + n");
            log && log(hex_view(str.data(), str.data() + str.size(), true));

            REQUIRE(str == std::string("\\\\n"));
        }
    }

    TEST_CASE("sstream.quot.1bslash", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.quot.1bslash"));

        log && log("quot(\"\\\")=[", quot("\\"), "]");

        std::stringstream ss2;
        ss2 << quot("\\");

        std::string str = ss2.str();

        REQUIRE(str == std::string("\"\\\\\"")); /* ["\\"] */
    }

    TEST_CASE("sstream.quot.newline", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.quot.newline"));

        log && log("quot(\"\\n\")=[", quot("\n"), "]");

        std::stringstream ss2;
        ss2 << quot("\n");

        std::string str = ss2.str();

        REQUIRE(str == std::string("\"\\n\"")); /* ["\n"] */
    }

    TEST_CASE("sstream.quot.2bslash", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quot.2bslash"));

        log && log("quot(\"\\\\\")=[", quot("\\\\"), "]");

        std::stringstream ss2;
        ss2 << quot("\\\\"); /* quoting string with two backslashes need to give ["\\\\"] */

        std::string str = ss2.str();

        REQUIRE(str == std::string("\"\\\\\\\\\"")); /* rhs is ["\\\\"] */
    }

    TEST_CASE("sstream.quot.2charnewline", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.quot.2charnewline"));

        log && log("quot(\"x\\n\")=[", quot("x\n"), "]");

        std::stringstream ss2;
        ss2 << quot("x\n");

        std::string str = ss2.str();

        REQUIRE(str == std::string("\"x\\n\"")); /* ["\n"] */
    }

    TEST_CASE("sstream.quot.2char", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.quot.2char"));

        log && log("quot(\"\\n\")=[", quot("\\n"), "]");

        std::stringstream ss2;
        ss2 << quot("\\n");

        std::string str = ss2.str();

        //std::cerr << quoted_debug::s_log_last_quoted.view() << std::endl;

        //log && log("debug_log=[", quoted_debug::s_log_last_quoted.view() , "]");

        REQUIRE(str == std::string("\"\\\\n\""));
    }

    TEST_CASE("sstream.quot.foonewline", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream.quot.2charnewline"));

        std::stringstream ss2;
        ss2 << quot("foo\n");

        std::string str = ss2.str();

        REQUIRE(str == std::string("\"foo\\n\"")); /* ["\n"] */
    }

    TEST_CASE("sstream.rest", "[quot]") {
        constexpr bool c_debug_flag = false;

        scope log(XO_DEBUG2(c_debug_flag, "TEST_CASE.sstream"));

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << quot("\n");

            std::string str = ss.str();

            log && log("after: quot('\\n')");
            log && log(hex_view(str.data(), str.data() + str.size(), true));
        }

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << quot("foo\n");

            std::string str = ss.str();

            log && log("after: quot(\"foo\n\")");
            log && log(hex_view(str.data(), str.data() + str.size(), true));
            log && log("> ss.str ----------------");
            log && log(ss.str());
            log && log("< ss.str ----------------");
        }

        /* testing unexpected sstream behavior */
        {
            std::stringstream ss;

            log && log("empty stream");

            ss << unq("\n");

            std::string str = ss.str();

            log && log("after: unq('\\n')");
            log && log(hex_view(str.data(), str.data() + str.size(), true));
        }

    } /*TEST_CASE(sstream)*/

    std::vector<quot_tcase> s_quot_tcase_v(
        {
            quot_tcase("", true, "\"\""),
            quot_tcase("", false, "\"\""),

            quot_tcase("foo", true, "foo"),
            quot_tcase("foo", false, "\"foo\""),

            quot_tcase("foo\n", true, "\"foo\\n\""),
            quot_tcase("foo\n", false, "\"foo\\n\""),

            quot_tcase("two words", true, "\"two words\""),
            quot_tcase("two words", false, "\"two words\""),

            quot_tcase("1st\n2nd", true, "\"1st\\n2nd\""),
            quot_tcase("1st\n2nd", false, "\"1st\\n2nd\""),

            quot_tcase("misakte\rfix", true, "\"misakte\\rfix\""),
            quot_tcase("misakte\rfix", false, "\"misakte\\rfix\""),

            quot_tcase("\"oh!\", she said", true, "\"\\\"oh!\\\", she said\""),
            quot_tcase("\"oh!\", she said", false, "\"\\\"oh!\\\", she said\""),

            // special carveout for strings bracketed by <..>;  assume already well-formed
            quot_tcase("<object printer output>", true, "<object printer output>"),
            quot_tcase("<object printer output>", false, "<object printer output>"),
        });

    TEST_CASE("quot", "[quot]") {
        for (std::uint32_t i_tc = 0, z_tc = s_quot_tcase_v.size(); i_tc < z_tc; ++i_tc) {
            quot_tcase const & tc = s_quot_tcase_v[i_tc];

            /* NOTE: don't use tag()/xtag() here,
             *       since implementation relies on the inserter we are testing
             */

            INFO(tostr("i_tc=", i_tc, " unq_flag=", tc.unq_flag_));
            INFO("tc.x_ ----------------");
            INFO(tostr("[", tc.x_, "]"));
            INFO("tc.x_ ----------------");

            std::stringstream ss;
            if (tc.unq_flag_)
                ss << unq(tc.x_);
            else
                ss << quot(tc.x_);

            std::string str = ss.str();

            INFO("tc.s ----------------");
            INFO(tostr("[", tc.s_, "]"));
            INFO("tc.s ----------------");
            INFO("ss.str ----------------");
            INFO(tostr("[", hex_view(str.data(), str.data() + str.size(), true), "]"));
            INFO(tostr("[", ss.str(), "]"));
            INFO("ss.str ----------------");

            REQUIRE(ss.str() == tc.s_);

            if (ss.str() != tc.s_)
                break;
        }

        REQUIRE(s_quot_tcase_v.size() > 1);
    }
} /*namespace ut*/

/* end quoted.test.cpp */
