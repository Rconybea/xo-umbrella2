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

    TEST_CASE("tag_ostream-fallback-value", "[tag_ostream]") {
        /* a double has no Prettifier => reaches the pretty() operator<< fallback,
         * which the FlatSink routes back to the ostream.  This is the case the
         * bridge exists for.
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
} /*namespace ut*/

/* end tag_ostream.test.cpp */
