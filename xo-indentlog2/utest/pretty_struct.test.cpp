/** @file pretty_struct.test.cpp
 *
 *  exercise PpSink::pretty_struct (xo/ppsink/pretty_struct.hpp) through the
 *  real line-breaking sink.
 *
 *  xo-ppsink's own pretty_struct tests can only pin the flat text and the
 *  token stream -- FlatSink's begin/split/end are no-ops, and PrettySink lives
 *  here.  So this is the only place the *rendered wrapped form* can be
 *  asserted, and in particular the indent it wraps to.
 *
 *  That indent is the point.  begin(offset) and split(spaces,offset) COMPOUND
 *  in PpState (begin adds to the running indent; a split's break target is
 *  running-indent + its own offset).  A printer that passes the indent to both
 *  wraps to twice the intended column, and nothing in the flat case reveals
 *  it.  These tests fail if pretty_struct regresses that way.
 **/

#include <xo/ppsink/pretty_struct.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::field;
    using xo::mm::ArenaConfig;

    namespace {
        /** render fields as a struct named @p name, at a given right margin.
         *
         *  NB the arena name must be unique per call: two PrettySinks sharing
         *  an ArenaConfig name interfere, and the symptom is wrong indentation
         *  in whichever case runs second -- which looks exactly like a bug in
         *  the printer under test.  Hence the counter.
         **/
        template <typename... Fields>
        std::string
        render(std::uint32_t margin, std::string_view name, const Fields &... fields)
        {
            static int seq = 0;

            ArenaConfig logbuf_cfg {
                .name_ = "utest.pretty_struct." + std::to_string(++seq),
                .size_ = 64*1024 };

            PpConfig cfg = PpConfig().with_logbuf_config(logbuf_cfg)
                                     .with_soft_right_margin(margin);

            PrettySink pp(cfg, nullptr);

            pp.pretty_struct(name, fields...);

            return std::string(pp.output());
        }

        /** a struct-shaped value that itself prints via pretty_struct,
         *  to exercise nesting
         **/
        struct Inner { int x_ = 1; };
    } /*namespace*/
} /*namespace ut*/

namespace xo::pp {
    template <>
    struct Prettifier<ut::Inner> {
        static void print(PpSink & sink, const ut::Inner & v) {
            sink.pretty_struct("In", field("x", v.x_));
        }
    };
} /*namespace xo::pp*/

namespace ut {
    TEST_CASE("pretty_struct-fits-one-line", "[pretty_struct]") {
        int alpha = 1, beta = 2;

        REQUIRE(render(135, "Point", field("alpha", alpha), field("beta", beta))
                == "<Point :alpha 1 :beta 2>");
    }

    TEST_CASE("pretty_struct-wraps-to-indent-2", "[pretty_struct]") {
        /* THE regression test.  indent_width defaults to 2, and pretty_struct
         * calls the no-arg begin() + split(1) -- so a broken field lands at
         * column 2.  If it ever lands at column 4, someone has passed the
         * indent to begin() AND split() and they have compounded.
         *
         * margin 20: wide enough that each field still fits on its own line
         * (so the only breaks are the ones between fields), narrow enough that
         * the 24-char flat form does not.
         */
        int alpha = 1, beta = 2;

        std::string out = render(20, "Point", field("alpha", alpha), field("beta", beta));

        REQUIRE(out == "<Point\n  :alpha 1\n  :beta 2>");

        /* stated separately so a regression reads unambiguously */
        REQUIRE(out.find("\n  :alpha") != std::string::npos);
        REQUIRE(out.find("\n    :alpha") == std::string::npos);
    }

    TEST_CASE("pretty_struct-nested-accumulates-indent", "[pretty_struct]") {
        /* begin() takes its offset from the sink's PpConfig and accumulates,
         * so an inner struct's fields sit one level deeper than the outer's:
         * outer fields at column 2, the inner struct's own field at 4.
         */
        Inner inner;
        int alpha = 1;

        std::string out = render(14, "Out", field("alpha", alpha), field("in", inner));

        INFO(out);
        REQUIRE(out == "<Out\n  :alpha 1\n  :in\n   <In\n    :x\n     1>>");

        /* the accumulation itself, stated directly */
        REQUIRE(out.find("\n  :alpha") != std::string::npos);   /* outer: 2 */
        REQUIRE(out.find("\n    :x") != std::string::npos);     /* inner: 4 */
    }

    TEST_CASE("pretty_struct-nested-fits-flat", "[pretty_struct]") {
        Inner inner;
        int alpha = 1;

        REQUIRE(render(135, "Out", field("alpha", alpha), field("in", inner))
                == "<Out :alpha 1 :in <In :x 1>>");
    }

    TEST_CASE("pretty_struct-absent-field-wrapped", "[pretty_struct]") {
        /* an omitted field must not leave a break opportunity behind: at a
         * margin where "<Point :alpha 1>" fits, the result stays flat.
         */
        int alpha = 1, beta = 2;

        REQUIRE(render(20, "Point", field("alpha", alpha), field("beta", beta, false))
                == "<Point :alpha 1>");
    }
} /*namespace ut*/

/* end pretty_struct.test.cpp */
