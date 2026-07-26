/* @file Pretty.test.cpp */

#include <xo/ppsink/Pretty.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <catch2/catch.hpp>
#include <ostream>
#include <sstream>

using xo::print::FlatSink;
using xo::print::PpSink;
using xo::print::Pretty;
using xo::print::pp_write;
using xo::print::has_pretty;

/* a type that opts in to Pretty */
struct Point_PpsinkTest { int x; int y; };

/* a type that does NOT opt in -> pp_write falls back to operator<< */
struct Plain_PpsinkTest { int v; };

inline std::ostream &
operator<<(std::ostream & os, const Plain_PpsinkTest & p) {
    return os << "Plain{" << p.v << "}";
}

namespace xo::print {
    template <>
    struct Pretty<Point_PpsinkTest> {
        static void print(PpSink & sink, const Point_PpsinkTest & p) {
            sink.begin().put("(");
            pp_write(sink, p.x);
            sink.put(",");
            pp_write(sink, p.y);
            sink.put(")").end();
        }
    };
}

TEST_CASE("pp_write-uses-specialization", "[pretty]") {
    static_assert(has_pretty<Point_PpsinkTest>);

    std::stringstream ss;
    FlatSink sink(ss);
    pp_write(sink, Point_PpsinkTest{1, 2});   /* begin/end are no-ops in FlatSink */

    REQUIRE(ss.str() == "(1,2)");
}

TEST_CASE("pp_write-falls-back-to-operator<<", "[pretty]") {
    static_assert(!has_pretty<Plain_PpsinkTest>);

    std::stringstream ss;
    FlatSink sink(ss);
    pp_write(sink, Plain_PpsinkTest{7});      /* stream_open -> operator<< */

    REQUIRE(ss.str() == "Plain{7}");
}
