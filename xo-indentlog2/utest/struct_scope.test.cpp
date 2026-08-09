/** @file struct_scope.test.cpp
 *
 *  exercise PpSink::struct_open() (xo/ppsink/pretty_struct.hpp) through the
 *  real line-breaking sink.
 *
 *  Two things can only be asserted here, not in xo-ppsink/utest:
 *
 *  1. that a dynamically-built struct wraps to the SAME indent as a variadic
 *     one.  begin(offset) and split(spaces,offset) compound in PpState, so a
 *     builder that passed the indent to both would wrap to twice the intended
 *     column -- invisible in the flat text and in the token stream.
 *
 *  2. that force_break actually forces.  newline()'s contract is "forcing
 *     every enclosing group to break"; whether that propagates through the
 *     enclosing groups is a rendering property.  The ppsink-side test can only
 *     see that a newline token was emitted, not what it did.
 **/

#include <xo/ppsink/pretty_struct.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::pp::PpSink;
    using xo::pp::field;
    using xo::mm::ArenaConfig;

    namespace {
        /** render whatever @p fn writes, at right margin @p margin.
         *
         *  NB unique arena name per call -- see the note in
         *  pretty_struct.test.cpp; sharing one produces wrong indentation in
         *  whichever case runs second.
         **/
        template <typename Fn>
        std::string render(std::uint32_t margin, Fn && fn) {
            static int seq = 0;

            ArenaConfig logbuf_cfg {
                .name_ = "utest.struct_scope." + std::to_string(++seq),
                .size_ = 64*1024 };

            PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg)
                                     .with_soft_right_margin(margin)
                                     .with_style(PpStyle::plain());

            PrettySink pp(cfg, nullptr);

            fn(pp);

            return std::string(pp.output());
        }
    } /*namespace*/

    /* ------------------------------------------------------------------
     * indent: a builder-built struct must wrap exactly like a variadic one
     */
    TEST_CASE("struct_scope-wraps-to-indent-2", "[struct_scope]") {
        const int alpha = 1;
        const int beta = 2;

        std::string out = render(12, [&alpha, &beta](PpSink & sink) {
            auto st = sink.struct_open("Point");
            st.field("alpha", alpha);
            st.field("beta", beta);
        });

        REQUIRE(out == "<Point\n  :alpha 1\n  :beta 2>");

        /* indent is 2, not 4 -- the compounding bug */
        REQUIRE(out.find("\n  :alpha") != std::string::npos);
        REQUIRE(out.find("\n    :alpha") == std::string::npos);
    }

    TEST_CASE("struct_scope-indent-matches-pretty_struct", "[struct_scope]") {
        const int alpha = 1;
        const int beta = 2;

        std::string via_builder = render(12, [&alpha, &beta](PpSink & sink) {
            auto st = sink.struct_open("Point");
            st.field("alpha", alpha);
            st.field("beta", beta);
        });
        std::string via_variadic = render(12, [&alpha, &beta](PpSink & sink) {
            sink.pretty_struct("Point", field("alpha", alpha), field("beta", beta));
        });

        REQUIRE(via_builder == via_variadic);
    }

    TEST_CASE("struct_scope-fits-one-line", "[struct_scope]") {
        const int alpha = 1;
        const int beta = 2;

        REQUIRE(render(135, [&alpha, &beta](PpSink & sink) {
            auto st = sink.struct_open("Point");
            st.field("alpha", alpha);
            st.field("beta", beta);
        }) == "<Point :alpha 1 :beta 2>");
    }

    /* ------------------------------------------------------------------
     * force_break: breaks even at a margin wide enough to fit.
     * This is the assertion that exists only here.
     */
    TEST_CASE("struct_scope-force-break-breaks-when-it-would-fit", "[struct_scope]") {
        const std::size_t size = 2;
        const int a = 7;
        const int b = 8;

        auto build = [&size, &a, &b](bool force) {
            return [&size, &a, &b, force](PpSink & sink) {
                auto st = sink.struct_open("stack", force);
                st.field("size", size);
                st.field("[1]", a);
                st.field("[0]", b);
            };
        };

        /* margin 135: comfortably fits on one line */
        REQUIRE(render(135, build(false)) == "<stack :size 2 :[1] 7 :[0] 8>");

        /* same margin, force_break: must break anyway */
        std::string forced = render(135, build(true));

        REQUIRE(forced == "<stack\n  :size 2\n  :[1] 7\n  :[0] 8>");
    }

    /* a forced break inside a nested struct forces the ENCLOSING group open
     * too -- newline()'s documented contract, and unobservable flat.
     */
    TEST_CASE("struct_scope-force-break-propagates-outward", "[struct_scope]") {
        const int x = 1;
        const int y = 2;

        std::string out = render(135, [&x, &y](PpSink & sink) {
            auto outer = sink.struct_open("Out");   /* no force here */
            outer.field("a", x);
            {
                /* inner forces: the outer group must break as well */
                auto inner = sink.struct_open("In", true /*force_break*/);
                inner.field("b", y);
            }
        });

        /* outer broke even though everything would have fitted at margin 135 */
        REQUIRE(out.find('\n') != std::string::npos);
        REQUIRE(out.find("<Out\n") != std::string::npos);
    }

    /* ------------------------------------------------------------------
     * dynamic arity: a loop-built struct renders like a hand-listed one
     */
    TEST_CASE("struct_scope-loop-wraps-correctly", "[struct_scope]") {
        const std::vector<int> v = {10, 20, 30};

        std::string out = render(12, [&v](PpSink & sink) {
            auto st = sink.struct_open("V");
            for (std::size_t i = 0; i < v.size(); ++i)
                st.field(std::string("[") + std::to_string(i) + "]", v[i]);
        });

        REQUIRE(out == "<V\n  :[0] 10\n  :[1] 20\n  :[2] 30>");
    }

    /* nested builders accumulate indent: inner fields sit one level deeper */
    TEST_CASE("struct_scope-nested-accumulates-indent", "[struct_scope]") {
        const int alpha = 1;
        const int x = 1;

        std::string out = render(12, [&alpha, &x](PpSink & sink) {
            auto outer = sink.struct_open("Out");
            outer.field("alpha", alpha);
            {
                auto inner = sink.struct_open("In");
                inner.field("x", x);
            }
        });

        /* outer fields at 2 */
        REQUIRE(out.find("\n  :alpha") != std::string::npos);
        /* inner field deeper than the outer one */
        REQUIRE(out.find("\n    :x") != std::string::npos);
    }
} /*namespace ut*/

/* end struct_scope.test.cpp */
