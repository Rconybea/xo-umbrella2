/** @file pad.test.cpp
 *
 *  exercise pad() / spaces() from xo/ppsink/pad.hpp, and the ostream bridge
 *  in pad_ostream.hpp.
 **/

#include <xo/ppsink/pad.hpp>
#include <xo/ppsink/pad_ostream.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::pad;
    using xo::pp::spaces;
    using std::stringstream;

    namespace {
        std::string pp_of(xo::pp::pad_impl x) {
            stringstream ss; FlatSink s(ss); s.pp(x); return ss.str();
        }
        std::string os_of(xo::pp::pad_impl x) {
            stringstream ss; ss << x; return ss.str();
        }
    } /*namespace*/

    TEST_CASE("pad-spaces", "[pad]") {
        REQUIRE(pp_of(pad(0)) == "");
        REQUIRE(pp_of(pad(1)) == " ");
        REQUIRE(pp_of(pad(8)) == "        ");
        REQUIRE(pp_of(spaces(4)) == "    ");
    }

    TEST_CASE("pad-explicit-char", "[pad]") {
        REQUIRE(pp_of(pad(16, '-')) == std::string(16, '-'));
        REQUIRE(pp_of(pad(3, '.')) == "...");
    }

    TEST_CASE("pad-longer-than-one-chunk", "[pad]") {
        /* the Prettifier writes in 64-char chunks; make sure a run longer than
         * one chunk comes out whole and at the right length
         */
        const std::uint32_t n = 200;

        std::string out = pp_of(pad(n, 'x'));

        REQUIRE(out.size() == n);
        REQUIRE(out == std::string(n, 'x'));
    }

    TEST_CASE("pad-ostream-bridge", "[pad]") {
        /* must agree byte-for-byte with the sink path */
        REQUIRE(os_of(pad(8)) == pp_of(pad(8)));
        REQUIRE(os_of(pad(16, '-')) == pp_of(pad(16, '-')));
        REQUIRE(os_of(pad(200, 'x')) == pp_of(pad(200, 'x')));

        /* the legacy idiom this replaces */
        stringstream ss;
        ss << ":" << pad(8) << ":";
        REQUIRE(ss.str() == ":        :");
    }
} /*namespace ut*/

/* end pad.test.cpp */
