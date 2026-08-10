/** @file tag_ostream.test.cpp
 *
 *  exercise the ostream bridge: operator<<(ostream, tag_impl) from
 *  xo/ppsink/tag_ostream.hpp -- the migration path for legacy `os << xtag(..)`.
 **/

#include <xo/ppsink/tag_ostream.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::tag;
    using xo::pp::xtag;
    using std::stringstream;

    TEST_CASE("tag_ostream-xtag", "[tag_ostream]") {
        /* xtag => one leading space, then ":name value" */
        stringstream ss;
        ss << xtag("k", 42);
        REQUIRE(ss.str() == " :k 42");
    }

    TEST_CASE("tag_ostream-tag", "[tag_ostream]") {
        /* tag => no leading space */
        stringstream ss;
        ss << tag("k", 42);
        REQUIRE(ss.str() == ":k 42");
    }

    /* see the identical helper in tostr.test.cpp: a double no longer reaches
     * the fallback (Prettifier<double>, 2026-08-09), so a tag value that
     * genuinely does has to be a type with an operator<< and no Prettifier<>.
     */
    struct FallbackOnly_TagTest { int v; };

    inline std::ostream &
    operator<<(std::ostream & os, const FallbackOnly_TagTest & x) {
        return os << "F(" << x.v << ")";
    }

    TEST_CASE("tag_ostream-fallback-value", "[tag_ostream]") {
        /* the tag value reaches pretty()'s operator<< fallback, which the
         * FlatSink routes back to the ostream.  This is the case the bridge
         * exists for.
         */
        static_assert(!xo::pp::has_prettifier<FallbackOnly_TagTest>);

        stringstream ss;
        ss << xtag("x", FallbackOnly_TagTest{3});
        REQUIRE(ss.str() == " :x F(3)");
    }

    TEST_CASE("tag_ostream-double-value", "[tag_ostream]") {
        /* a double now goes through Prettifier<double> instead, and the bridge
         * still produces the same bytes -- which is the property that matters.
         */
        stringstream ss;
        ss << xtag("x", 2.5);
        REQUIRE(ss.str() == " :x 2.5");
    }

    TEST_CASE("tag_ostream-chained", "[tag_ostream]") {
        /* the pervasive display() idiom: literal + several tags, one statement */
        stringstream ss;
        ss << "<obj" << xtag("a", 1) << xtag("b", 2) << ">";
        REQUIRE(ss.str() == "<obj :a 1 :b 2>");
    }
    /* XTAG_(x) names the tag after the *expression*, mirroring legacy
     * XTAG(x) = xo::xtag(STRINGIFY(x), x).  Trailing underscore per the
     * ppsink convention, so a TU can also pull in legacy indentlog's XTAG.
     */
    TEST_CASE("tag_ostream-XTAG_", "[tag_ostream]") {
        int width = 7;
        stringstream ss;
        ss << XTAG_(width);
        REQUIRE(ss.str() == " :width 7");
    }

    TEST_CASE("tag_ostream-XTAG_-expression", "[tag_ostream]") {
        /* the name is the token text, verbatim */
        int n = 2;
        stringstream ss;
        ss << XTAG_(n + 1);
        REQUIRE(ss.str() == " :n + 1 3");
    }

} /*namespace ut*/

/* end tag_ostream.test.cpp */
