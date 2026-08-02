/* @file Prettifier.test.cpp */

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp> /* Plain_PpsinkTest exercises the operator<< fallback */
#include <catch2/catch.hpp>
#include <sstream>

using xo::pp::FlatSink;
using xo::pp::PpSink;
using xo::pp::Prettifier;
using xo::pp::pretty;
using xo::pp::has_prettifier;

/* a type that opts in to Prettifier */
struct Point_PpsinkTest { int x; int y; };

/* a type that does NOT opt in -> pretty falls back to operator<< */
struct Plain_PpsinkTest { int v; };

inline std::ostream &
operator<<(std::ostream & os, const Plain_PpsinkTest & p) {
    return os << "Plain{" << p.v << "}";
}

namespace xo::pp {
    template <>
    struct Prettifier<Point_PpsinkTest> {
        static void print(PpSink & sink, const Point_PpsinkTest & p) {
            sink.begin().put("(");
            pretty(sink, p.x);
            sink.put(",");
            pretty(sink, p.y);
            sink.put(")").end();
        }
    };
}

TEST_CASE("pretty-uses-specialization", "[pretty]") {
    static_assert(has_prettifier<Point_PpsinkTest>);

    std::stringstream ss;
    FlatSink sink(ss);
    pretty(sink, Point_PpsinkTest{1, 2});   /* begin/end are no-ops in FlatSink */

    REQUIRE(ss.str() == "(1,2)");
}

TEST_CASE("pretty-falls-back-to-operator<<", "[pretty]") {
    static_assert(!has_prettifier<Plain_PpsinkTest>);

    std::stringstream ss;
    FlatSink sink(ss);
    pretty(sink, Plain_PpsinkTest{7});      /* stream_open -> operator<< */

    REQUIRE(ss.str() == "Plain{7}");
}
