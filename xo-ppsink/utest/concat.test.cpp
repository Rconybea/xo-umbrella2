/** @file concat.test.cpp
 *
 *  exercise concat() from xo/ppsink/concat.hpp.
 *
 *  Two things worth pinning beyond "the text comes out right":
 *
 *  - concat emits NO structure of its own (no begin/split/end), so it is safe
 *    in a position that must stay one unbreakable token -- e.g. a field name.
 *    Checked against the token stream, where FlatSink would hide it.
 *  - it does not copy its arguments; that is the reason it exists.
 **/

#include <xo/ppsink/concat.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/tostr.hpp>
#include <xo/ppsink/pretty_ostream.hpp>
#include "MarkSink.hpp"
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::concat;
    using std::stringstream;

    namespace {
        template <typename Fn>
        std::string flat_of(Fn && fn) {
            stringstream ss; FlatSink sink(ss.rdbuf()); fn(sink); return ss.str();
        }
        template <typename Fn>
        std::string marks_of(Fn && fn) {
            stringstream ss; MarkSink sink(ss); fn(sink); return ss.str();
        }

        /** counts copies, to pin that concat captures by reference **/
        struct Watched {
            Watched() = default;
            Watched(const Watched &) { ++n_copy_; }
            Watched & operator=(const Watched &) { ++n_copy_; return *this; }
            static int n_copy_;
        };
        int Watched::n_copy_ = 0;

        inline std::ostream & operator<<(std::ostream & os, const Watched &) {
            os << "W"; return os;
        }
    } /*namespace*/

    TEST_CASE("concat-basic", "[concat]") {
        const int i = 7;

        REQUIRE(flat_of([&i](xo::pp::PpSink & sink) {
            sink.pp(concat("[", i, "]"));
        }) == "[7]");
    }

    TEST_CASE("concat-single-and-many", "[concat]") {
        const int a = 1, b = 2, c = 3;

        REQUIRE(flat_of([&a](xo::pp::PpSink & sink) { sink.pp(concat(a)); }) == "1");
        REQUIRE(flat_of([&a,&b,&c](xo::pp::PpSink & sink) {
            sink.pp(concat("<", a, ",", b, ",", c, ">"));
        }) == "<1,2,3>");
    }

    /* the reason it exists: byte-identical to the tostr() idiom it replaces,
     * without building a std::string
     */
    TEST_CASE("concat-matches-tostr", "[concat]") {
        for (int i : {0, 7, 42, 1234}) {
            std::string via_concat = flat_of([&i](xo::pp::PpSink & sink) {
                sink.pp(concat("[", i, "]"));
            });
            REQUIRE(via_concat == xo::pp::tostr("[", i, "]"));
        }
    }

    /* concat must contribute no group structure of its own: it has to be safe
     * where the output must stay one unbreakable token.  FlatSink cannot show
     * this -- its begin/split/end are no-ops -- so check the token stream.
     */
    TEST_CASE("concat-emits-no-structure", "[concat]") {
        const int i = 7;

        std::string marks = marks_of([&i](xo::pp::PpSink & sink) {
            sink.pp(concat("[", i, "]"));
        });

        REQUIRE(marks == "[7]");
        REQUIRE(marks.find("<G") == std::string::npos);
        REQUIRE(marks.find("<S ") == std::string::npos);
        REQUIRE(marks.find("<N ") == std::string::npos);
    }

    TEST_CASE("concat-does-not-copy", "[concat]") {
        Watched w;
        const int before = Watched::n_copy_;

        REQUIRE(flat_of([&w](xo::pp::PpSink & sink) {
            sink.pp(concat("(", w, ")"));
        }) == "(W)");

        REQUIRE(Watched::n_copy_ == before);
    }

    /* a prvalue argument lives to the end of the full-expression, so building
     * concat in the expression that consumes it is safe
     */
    TEST_CASE("concat-prvalue-argument", "[concat]") {
        const int i = 20;

        REQUIRE(flat_of([&i](xo::pp::PpSink & sink) {
            sink.pp(concat("[", i + 1, "]"));
        }) == "[21]");
    }

    TEST_CASE("concat-nested", "[concat]") {
        const int a = 1, b = 2;

        REQUIRE(flat_of([&a,&b](xo::pp::PpSink & sink) {
            sink.pp(concat("<", concat(a, ":", b), ">"));
        }) == "<1:2>");
    }

    /* std::string and string_view arguments render as their text, not through
     * any quoting path
     */
    TEST_CASE("concat-string-arguments", "[concat]") {
        const std::string s = "abc";
        const std::string_view sv = "def";

        REQUIRE(flat_of([&s,&sv](xo::pp::PpSink & sink) {
            sink.pp(concat(s, "-", sv));
        }) == "abc-def");
    }
} /*namespace ut*/

/* end concat.test.cpp */
