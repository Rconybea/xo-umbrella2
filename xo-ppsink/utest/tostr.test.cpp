/** @file tostr.test.cpp
 *
 *  exercise xo::pp::tostr (from xo/ppsink/tostr.hpp) -- the flat, FlatSink-over-
 *  stringstream string builder that replaces legacy xo::tostr.
 **/

#include <xo/ppsink/tostr.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace ut {
    using xo::pp::tostr;

    TEST_CASE("tostr-basic", "[tostr]") {
        REQUIRE(tostr() == "");                       /* empty pack */
        REQUIRE(tostr("hello") == "hello");           /* single string leaf */
        REQUIRE(tostr("a", "b", "c") == "abc");       /* concatenation, no separator */
    }

    TEST_CASE("tostr-mixed", "[tostr]") {
        REQUIRE(tostr("x=", 42) == "x=42");           /* string + int (Prettifier<int>) */
        REQUIRE(tostr(1, 2, 3) == "123");
    }

    TEST_CASE("tostr-fallback", "[tostr]") {
        /* a double has no Prettifier => operator<< fallback via the FlatSink */
        REQUIRE(tostr("v=", 2.5) == "v=2.5");
    }
} /*namespace ut*/

/* end tostr.test.cpp */
