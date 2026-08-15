/** @file tostr.test.cpp
 *
 *  exercise xo::pp::tostr0 (from xo/ppsink/tostr0.hpp)
 **/

#include <xo/ppsink/tostr0.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace ut {
    using xo::pp::tostr0;

    TEST_CASE("tostr-basic", "[tostr]") {
        REQUIRE(tostr0() == "");                       /* empty pack */
        REQUIRE(tostr0("hello") == "hello");           /* single string leaf */
        REQUIRE(tostr0("a", "b", "c") == "abc");       /* concatenation, no separator */
    }

    TEST_CASE("tostr-mixed", "[tostr]") {
        REQUIRE(tostr0("x=", 42) == "x=42");           /* string + int (Prettifier<int>) */
        REQUIRE(tostr0(1, 2, 3) == "123");
    }

    /* a type with an operator<< and no Prettifier<> -- the only way left to
     * reach pretty()'s third dispatch branch.  It used to be enough to pass a
     * double here, but Prettifier<double> (2026-08-09) and the integer/bool
     * specializations (2026-08-10) mean the scalars no longer go near it.
     */
    struct FallbackOnly_TostrTest { int v; };

    inline std::ostream &
    operator<<(std::ostream & os, const FallbackOnly_TostrTest & x) {
        return os << "F(" << x.v << ")";
    }

    TEST_CASE("tostr-fallback", "[tostr]") {
        static_assert(!xo::pp::has_prettifier<FallbackOnly_TostrTest>);

        REQUIRE(tostr0("v=", FallbackOnly_TostrTest{3}) == "v=F(3)");
    }

    TEST_CASE("tostr-double", "[tostr]") {
        /* NOT the fallback any more: Prettifier<double> owns this, and renders
         * to_chars %.6g -- which is exactly what operator<< used to produce.
         */
        static_assert(xo::pp::has_prettifier<double>);

        REQUIRE(tostr0("v=", 2.5) == "v=2.5");
    }
} /*namespace ut*/

/* end tostr.test.cpp */
