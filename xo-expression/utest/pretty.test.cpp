/** @file pretty.test.cpp
 *
 *  Pin the rendered output of Expression::pretty(PpSink&), which nothing did
 *  before the ppindentinfo -> PpSink conversion: xo-expression had exactly one
 *  utest (type_unifier) and it asserts nothing about printing.
 *
 *  Scoped to the shapes that carry risk rather than every expression type:
 *
 *   - Sequence      -- dynamic arity, built with sink.struct_open()
 *   - LocalSymtab   -- the deliberate `this` field (see below)
 *   - Constant      -- plain pretty_struct, as a control on the common path
 *   - narrow margin -- that a group actually breaks, and at the right indent
 *
 *  Every expectation here was MEASURED against the built library, not
 *  predicted.  Layout has been mispredicted twice on this migration.
 **/

#include "xo/expression/Sequence.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/LocalSymtab.hpp"
#include "xo/expression/pretty_expression.hpp"
#include "xo/expression/pretty_localenv.hpp"
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <vector>

namespace ut {
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;
    using xo::pp::PpStyle;
    using xo::mm::ArenaConfig;
    using xo::scm::Expression;
    using xo::scm::Sequence;
    using xo::scm::Constant;
    using xo::scm::LocalSymtab;
    using xo::rp;

    /* the dispatch chain this file depends on: rp<Expression> -> Expression ->
     * the virtual pretty().  If either specialization stops being visible, the
     * render silently falls back to operator<< (i.e. display()), which cannot
     * line-break -- a failure easy to misread as a layout bug.
     */
    static_assert(xo::pp::has_prettifier<xo::scm::Expression>,
                  "Prettifier<Expression> must be visible");
    static_assert(xo::pp::has_prettifier<xo::rp<xo::scm::Expression>>,
                  "Prettifier<rp<Expression>> must be visible");

    namespace {
        /** render @p x at right margin @p margin.
         *
         *  NB unique arena name per call: two PrettySinks sharing an
         *  ArenaConfig name interfere, and the symptom is wrong indentation in
         *  whichever case runs second.
         **/
        template <typename T>
        std::string render(std::uint32_t margin, const T & x) {
            PrettySink pp = PrettySink::scratch("utest.expression.pretty.",
                                                64*1024, margin);
            pp.pp(x);
            return std::string(pp.output());
        }

        rp<Expression> mk_const(double v) { return Constant<double>::make(v); }

        rp<Expression> mk_seq2() {
            std::vector<rp<Expression>> v = { mk_const(1), mk_const(2) };
            return Sequence::make(v);
        }
    } /*namespace*/

    TEST_CASE("pretty-constant-flat", "[pretty]") {
        REQUIRE(render(200, mk_const(1.5))
                == "<Constant :type \"double\" :value 1.5>");
    }

    /* Sequence is the dynamic-arity case: its fields come from a loop through
     * sink.struct_open(), so this is what breaks if the builder regresses.
     * In particular the index labels must advance -- the legacy display() had
     * a bug here, emitting :[0] for every element.
     */
    TEST_CASE("pretty-sequence-flat", "[pretty]") {
        REQUIRE(render(200, mk_seq2())
                == "<Sequence"
                   " :[0] <Constant :type \"double\" :value 1>"
                   " :[1] <Constant :type \"double\" :value 2>>");
    }

    /* a narrow margin must actually break.  Fields land at indent 2; a broken
     * field's value at 3 (ppsink's tag value_offset).  The indent-compounding
     * bug would put these at 4 and 6.
     */
    TEST_CASE("pretty-sequence-wraps", "[pretty]") {
        REQUIRE(render(40, mk_seq2())
                == "<Sequence\n"
                   "  :[0]\n"
                   "   <Constant :type \"double\" :value 1>\n"
                   "  :[1]\n"
                   "   <Constant :type \"double\" :value 2>>");
    }

    /* narrower still: the inner Constant breaks too, one level deeper */
    TEST_CASE("pretty-sequence-wraps-nested", "[pretty]") {
        REQUIRE(render(20, mk_seq2())
                == "<Sequence\n"
                   "  :[0]\n"
                   "   <Constant\n"
                   "    :type \"double\"\n"
                   "    :value 1>\n"
                   "  :[1]\n"
                   "   <Constant\n"
                   "    :type \"double\"\n"
                   "    :value 2>>");
    }

    /* LocalSymtab prints `this`.  The legacy two-pass version disagreed with
     * itself -- the fit pass measured `<LocalSymtab :argv ...>` while the print
     * pass emitted `this` too, so the single-line form could overflow the
     * margin.  The print branch was the intended one; this pins that decision.
     */
    TEST_CASE("pretty-localsymtab-includes-this", "[pretty]") {
        rp<LocalSymtab> st = LocalSymtab::make_empty();

        std::string out = render(200, st);

        REQUIRE(out.find("<LocalSymtab") != std::string::npos);
        REQUIRE(out.find(":this") != std::string::npos);
        REQUIRE(out.find(":argv") != std::string::npos);
    }

    /* NB no empty-Sequence case: Sequence's constructor computes its valuetype
     * from xv[xv.size()-1], which underflows on an empty vector -- an empty
     * Sequence cannot be constructed at all.  Pre-existing, unrelated to
     * pretty().
     */
} /*namespace ut*/

/* end pretty.test.cpp */
