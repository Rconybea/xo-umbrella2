/** @file struct_scope.test.cpp
 *
 *  exercise PpSink::struct_open() + struct_scope from
 *  xo/ppsink/pretty_struct.hpp -- the builder for a struct whose field count
 *  is only known at runtime.
 *
 *  Assertions here are about the flat text and the *token stream*.  Whether a
 *  forced break actually propagates through enclosing groups is a rendering
 *  question, and lives in xo-indentlog2/utest/struct_scope.test.cpp -- ppsink
 *  has no PrettySink.
 **/

#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/pretty_ostream.hpp>
#include "MarkSink.hpp"
#include <catch2/catch.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace ut {
    using xo::pp::FlatSink;
    using xo::pp::field;
    using std::stringstream;
    /* MarkSink is declared in namespace ut by MarkSink.hpp */

    namespace {
        /** flat text produced by @p fn **/
        template <typename Fn>
        std::string flat_of(Fn && fn) {
            stringstream ss;
            FlatSink sink(ss.rdbuf());
            fn(sink);
            return ss.str();
        }

        /** token stream produced by @p fn **/
        template <typename Fn>
        std::string marks_of(Fn && fn) {
            stringstream ss;
            MarkSink sink(ss);
            fn(sink);
            return ss.str();
        }

        /** a struct-shaped type that renders itself with the builder,
         *  so nesting can be exercised
         **/
        struct Inner {
            int x_ = 1;
        };
    } /*namespace*/
} /*namespace ut*/

namespace xo::pp {
    template <>
    struct Prettifier<ut::Inner> {
        static void print(PpSink & sink, const ut::Inner & x) {
            auto st = sink.struct_open("In");
            st.field("x", x.x_);
        }
    };
} /*namespace xo::pp*/

namespace ut {

    /* ------------------------------------------------------------------
     * the load-bearing test: a fixed field list built through struct_open()
     * must be indistinguishable from pretty_struct(), token for token.
     * pretty_struct() is implemented in terms of struct_scope, so this pins
     * that the wrapper did not change the shape.
     */
    TEST_CASE("struct_scope-matches-pretty_struct", "[struct_scope]") {
        const int a = 1;
        const int b = 2;

        auto via_builder = [&a, &b](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.field("a", a);
            st.field("b", b);
        };
        auto via_variadic = [&a, &b](xo::pp::PpSink & sink) {
            sink.pretty_struct("P", field("a", a), field("b", b));
        };

        REQUIRE(flat_of(via_builder) == flat_of(via_variadic));
        REQUIRE(marks_of(via_builder) == marks_of(via_variadic));

        /* and the value itself, so a shared regression cannot hide */
        REQUIRE(flat_of(via_builder) == "<P :a 1 :b 2>");
    }

    TEST_CASE("struct_scope-no-fields", "[struct_scope]") {
        REQUIRE(flat_of([](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            (void)st;
        }) == "<P>");
    }

    /* a loop-contributed field must be indistinguishable from a variadic one:
     * this is the whole point of the builder.
     */
    TEST_CASE("struct_scope-loop-matches-variadic", "[struct_scope]") {
        const std::vector<int> v = {10, 20, 30};

        std::string looped = flat_of([&v](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("V");
            for (std::size_t i = 0; i < v.size(); ++i)
                st.field(std::string("[") + std::to_string(i) + "]", v[i]);
        });

        REQUIRE(looped == "<V :[0] 10 :[1] 20 :[2] 30>");
    }

    /* the closing ">" belongs INSIDE the group, before end().  Getting this
     * wrong renders the ">" on its own line once the group breaks.
     */
    TEST_CASE("struct_scope-token-stream", "[struct_scope]") {
        const int a = 1;

        REQUIRE(marks_of([&a](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.field("a", a);
        }) == "<P<G><S 1,0><G 0>:a<S 1,1>1</G>></G>");
    }

    /* begin() must take its indent from the sink, not from the caller */
    TEST_CASE("struct_scope-begin-takes-indent-from-sink", "[struct_scope]") {
        std::string marks = marks_of([](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            (void)st;
        });

        REQUIRE(marks.find("<G>") != std::string::npos);
        REQUIRE(marks.find("<G 2>") == std::string::npos);
    }

    /* ------------------------------------------------------------------
     * force_break: separator becomes newline() rather than split().
     */
    TEST_CASE("struct_scope-force-break-emits-newline", "[struct_scope]") {
        const int a = 1;
        const int b = 2;

        auto build = [&a, &b](bool force) {
            return [&a, &b, force](xo::pp::PpSink & sink) {
                auto st = sink.struct_open("P", force);
                st.field("a", a);
                st.field("b", b);
            };
        };

        /* optional break: <S 1,0> separators, no <N> anywhere */
        std::string relaxed = marks_of(build(false));
        REQUIRE(relaxed.find("<S 1,0>") != std::string::npos);
        REQUIRE(relaxed.find("<N ") == std::string::npos);

        /* forced break: <N 0> separators, and no field-level <S 1,0> */
        std::string forced = marks_of(build(true));
        REQUIRE(forced.find("<N 0>") != std::string::npos);
        REQUIRE(forced.find("<S 1,0>") == std::string::npos);

        /* FlatSink renders a forced break as a hard newline even flat */
        REQUIRE(flat_of(build(true)) == "<P\n:a 1\n:b 2>");
        REQUIRE(flat_of(build(false)) == "<P :a 1 :b 2>");
    }

    /* absent fields drop the field AND its separator, as with pretty_struct */
    TEST_CASE("struct_scope-absent-field-skipped", "[struct_scope]") {
        const int a = 1;
        const int b = 2;

        REQUIRE(flat_of([&a, &b](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.field("a", a);
            st.field("b", b, false /*present*/);
        }) == "<P :a 1>");

        /* no stray separator left behind by the absent field */
        std::string marks = marks_of([&a, &b](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.field("a", a);
            st.field("b", b, false /*present*/);
        });
        REQUIRE(marks == "<P<G><S 1,0><G 0>:a<S 1,1>1</G>></G>");
    }

    TEST_CASE("struct_scope-all-fields-absent", "[struct_scope]") {
        const int a = 1;

        REQUIRE(flat_of([&a](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.field("a", a, false /*present*/);
        }) == "<P>");
    }

    /* item() accepts an already-built field-like, honouring present() */
    TEST_CASE("struct_scope-item-accepts-field", "[struct_scope]") {
        const int a = 1;
        const int b = 2;

        REQUIRE(flat_of([&a, &b](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            st.item(field("a", a));
            st.item(field("b", b, false /*present*/));
        }) == "<P :a 1>");
    }

    /* a generated name is a temporary: it must survive to the end of the
     * field() call, which is all struct_scope needs (it renders immediately).
     */
    TEST_CASE("struct_scope-generated-name-survives", "[struct_scope]") {
        const int v = 7;

        REQUIRE(flat_of([&v](xo::pp::PpSink & sink) {
            auto st = sink.struct_open("P");
            for (int i = 0; i < 2; ++i)
                st.field(std::string("[") + std::to_string(i) + "]", v);
        }) == "<P :[0] 7 :[1] 7>");
    }

    /* nesting: a field whose value is itself rendered with a builder.
     * The inner begin()/end() must nest inside the outer group.
     */
    TEST_CASE("struct_scope-nested", "[struct_scope]") {
        const Inner in;

        REQUIRE(flat_of([&in](xo::pp::PpSink & sink) {
            auto outer = sink.struct_open("Out");
            outer.field("a", 1);
            outer.field("in", in);
        }) == "<Out :a 1 :in <In :x 1>>");

        /* inner group opens and closes inside the outer one */
        std::string marks = marks_of([&in](xo::pp::PpSink & sink) {
            auto outer = sink.struct_open("Out");
            outer.field("in", in);
        });
        REQUIRE(marks == "<Out<G><S 1,0><G 0>:in<S 1,1><In<G><S 1,0>"
                         "<G 0>:x<S 1,1>1</G>></G></G>></G>");
    }
} /*namespace ut*/

/* end struct_scope.test.cpp */
