/* @file Prettifier.test.cpp */

#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp> /* Plain_PpsinkTest exercises the operator<< fallback */
#include <xo/ppsink/tostr0.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <limits>
#include <sstream>

using xo::pp::FlatSink;
using xo::pp::PpSink;
using xo::pp::Prettifier;
using xo::pp::pretty;
using xo::pp::has_prettifier;
using xo::pp::tostr0;

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
    FlatSink sink(ss.rdbuf());
    pretty(sink, Point_PpsinkTest{1, 2});   /* begin/end are no-ops in FlatSink */

    REQUIRE(ss.str() == "(1,2)");
}

TEST_CASE("pretty-falls-back-to-operator<<", "[pretty]") {
    static_assert(!has_prettifier<Plain_PpsinkTest>);

    std::stringstream ss;
    FlatSink sink(ss.rdbuf());
    pretty(sink, Plain_PpsinkTest{7});      /* stream_open -> operator<< */

    REQUIRE(ss.str() == "Plain{7}");
}

/* Scalar leaves.
 *
 * Prettifier<int> used to be the only integer specialization, so every other
 * width -- notably std::uint32_t, which is what a container's size() usually
 * is -- fell through to the operator<< fallback: right answer, wrong path, and
 * silently.  These pin that widening as OUTPUT-NEUTRAL: each expectation below
 * was observed BEFORE the specializations existed and is unchanged after.
 *
 * See .xo-backlog/xo-ppsink/issues/09-scalar-prettifiers.md
 */

TEST_CASE("prettifier-integer-widths", "[pretty][scalar]") {
    static_assert(has_prettifier<short>);
    static_assert(has_prettifier<unsigned short>);
    static_assert(has_prettifier<int>);
    static_assert(has_prettifier<unsigned int>);
    static_assert(has_prettifier<long>);
    static_assert(has_prettifier<unsigned long>);
    static_assert(has_prettifier<long long>);
    static_assert(has_prettifier<unsigned long long>);
    /* the typedefs that motivated this: sizes and fixed-width ints */
    static_assert(has_prettifier<std::size_t>);
    static_assert(has_prettifier<std::uint32_t>);
    static_assert(has_prettifier<std::int64_t>);

    CHECK(tostr0(static_cast<short>(-5)) == "-5");
    CHECK(tostr0(static_cast<unsigned short>(5)) == "5");
    CHECK(tostr0(42) == "42");
    CHECK(tostr0(4000000000u) == "4000000000");
    CHECK(tostr0(-1234567890123L) == "-1234567890123");
    CHECK(tostr0(1234567890123UL) == "1234567890123");
    CHECK(tostr0(static_cast<std::size_t>(42)) == "42");

    /* the extremes: buf[24] must hold the longest rendering of any width */
    CHECK(tostr0(std::numeric_limits<std::int64_t>::min()) == "-9223372036854775808");
    CHECK(tostr0(std::numeric_limits<std::uint64_t>::max()) == "18446744073709551615");
}

TEST_CASE("prettifier-bool-stays-1-0", "[pretty][scalar]") {
    static_assert(has_prettifier<bool>);

    /* NOT "true"/"false".  operator<< prints 1/0 without std::boolalpha, and
     * renderings pinned across the tree already contain it -- e.g. TypeDescr's
     * ":complete 1".  Prettifier<bool> removes the ostream, not the format.
     */
    CHECK(tostr0(true) == "1");
    CHECK(tostr0(false) == "0");
}

TEST_CASE("prettifier-leaves-char-types-alone", "[pretty][scalar]") {
    /* char IS integral, so a bare std::integral constraint would have caught
     * these and turned 'A' into "65" everywhere in xo.  pp_number_integral
     * excludes them on purpose; they keep rendering as CHARACTERS via
     * operator<<.
     */
    static_assert(!has_prettifier<char>);
    static_assert(!has_prettifier<signed char>);
    static_assert(!has_prettifier<unsigned char>);
    /* std::int8_t/uint8_t are typedefs for these, hence also characters */
    static_assert(!has_prettifier<std::int8_t>);
    static_assert(!has_prettifier<std::uint8_t>);

    CHECK(tostr0('A') == "A");
    CHECK(tostr0(static_cast<signed char>(65)) == "A");
    CHECK(tostr0(static_cast<unsigned char>(65)) == "A");
}

/* Pointers: byte-identical to operator<<(std::ostream&, const void*),
 * except null, which xo renders "null".
 *
 * void* and const void* are checked separately because Prettifier<T> matches
 * exactly: covering one does not cover the other.  They were 307 and 52 of the
 * 776 fallback instantiations measured 2026-08-16 (see
 * .xo-backlog/xo-ppsink/issues/12-operator-fallback-inventory.md).
 */
TEST_CASE("prettifier-pointer", "[prettifier][pointer]") {
    static_assert(xo::pp::has_prettifier<void *>);
    static_assert(xo::pp::has_prettifier<const void *>);

    int v = 7;
    /* null is pinned below instead: xo renders it "null", so it is not
     * expected to match operator<< on any host
     */
    void * cases[] = { reinterpret_cast<void *>(1),
                       reinterpret_cast<void *>(0xff),
                       static_cast<void *>(&v) };

    for (void * p : cases) {
        const void * cp = p;

        std::stringstream expect_p, expect_cp;
        expect_p << p;
        expect_cp << cp;

        INFO("expect " + expect_p.str());

        CHECK(tostr0(p) == expect_p.str());
        CHECK(tostr0(cp) == expect_cp.str());
    }

    /* the shape, spelled out, so a change on either side is visible here */
    CHECK(tostr0(static_cast<void *>(nullptr)) == "null");
    CHECK(tostr0(static_cast<const void *>(nullptr)) == "null");
    CHECK(tostr0(reinterpret_cast<void *>(0xff)) == "0xff");
}
