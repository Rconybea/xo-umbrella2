/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-interpreter2's printers.
 *
 * Follows the template in xo-expression2/utest/printable_render.test.cpp and
 * xo-reader2/utest/printable_render.test.cpp -- see
 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md for why BOTH
 * renderings are pinned rather than only asserted equal.
 *
 * DIFFERENT FROM the reader2 file in the way that decides the work.  There the
 * ssms were reachable only through the parser, so ParserStack and
 * DSchematikaParser had to convert before anything nested inside them could be
 * observed at all.  Here every printer's owner is directly constructible: each
 * has a public `make(obj<AAllocator> mm, ...)` taking plain arguments, so a
 * test builds one itself.  Which is just as well -- DVirtualSchematikaMachine
 * has NO printer, and its stack_ is private with no accessor, so the reader2
 * route does not exist here.
 *
 * The frames also do not nest in output: each stores a parent/stack link, but
 * no printer renders it.  The `:cont` field every frame prints is a VsmInstr,
 * a leaf holding one vsm_opcode.  So the eight printers are nearly independent
 * -- only DClosure and DVsmApplyClosureFrame have a dependency, both on
 * DLocalEnv, which is why DLocalEnv converts first.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/interpreter2/Closure.hpp>
#include <xo/interpreter2/LocalEnv.hpp>
#include <xo/interpreter2/VsmApplyClosureFrame.hpp>
#include <xo/interpreter2/VsmApplyFrame.hpp>
#include <xo/interpreter2/VsmDefContFrame.hpp>
#include <xo/interpreter2/VsmEvalArgsFrame.hpp>
#include <xo/interpreter2/VsmIfElseContFrame.hpp>
#include <xo/interpreter2/VsmSeqContFrame.hpp>
#include <xo/interpreter2/init_interpreter2.hpp>
#include <xo/expression2/LambdaExpr.hpp>
#include <xo/expression2/LocalSymtab.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>

namespace xo {
    using xo::mm::ArenaConfig;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::scm::DClosure;
    using xo::scm::DLambdaExpr;
    using xo::scm::DLocalEnv;
    using xo::scm::DVsmApplyClosureFrame;
    using xo::scm::TypeRef;
    using xo::scm::DLocalSymtab;
    using xo::scm::DVsmApplyFrame;
    using xo::scm::DVsmDefContFrame;
    using xo::scm::DVsmEvalArgsFrame;
    using xo::scm::DVsmIfElseContFrame;
    using xo::scm::DVsmSeqContFrame;
    using xo::scm::VsmInstr;
    using xo::scm::DArray;
    using xo::scm::DInteger;
    using xo::print::APrintable;
    using xo::facet::obj;

    static InitEvidence s_init = InitSubsys<S_interpreter2_tag>::require();

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string
            render_pretty(const T & x, std::uint32_t margin) {
                return toppstr(PpConfig::scratch_plain(margin), x);
            }

            /** A plain arena, no collector.
             *
             *  The reader2 fixture needed a real X1 collector because the
             *  parser registers itself as a gc root.  Nothing here does: these
             *  are ordinary gc objects built directly and never collected
             *  within a test, so an arena is enough.  Copied in spirit from
             *  ArenaShim in VirtualSchematikaMachine.test.cpp.
             **/
            struct ArenaFixture {
                explicit ArenaFixture(const std::string & name,
                                      std::size_t size = 16*1024)
                    : arena_(ArenaConfig().with_name(name).with_size(size)) {}

                obj<AAllocator,DArena> mm() { return obj<AAllocator,DArena>(&arena_); }

                DArena arena_;
            };
        }

        /** DLocalEnv -- a one-field struct, and the first printer in this
         *  subsystem, so it is also what establishes the file.
         *
         *  The field is `:n_args`, which is args_->size() -- the SIZE of the
         *  argument array, not its capacity.  DArray::_empty(mm, cap) yields
         *  size 0 whatever cap is, so the multi-arg cases push_back().
         *
         *  NOT pinned: a null args_.  DLocalEnv::_make asserts symtab but NOT
         *  args, and the printer dereferences args_ unguarded
         *  (xo-interpreter2/src/interpreter2/DLocalEnv.cpp:121), so a null
         *  args_ segfaults on BOTH protocols.  Same family as
         *  .xo-backlog/xo-reader2/issues/01-ssm-printer-null-children.md;
         *  recorded there rather than fixed here, since this refactor is not
         *  meant to change behaviour.
         *
         *  Neither symtab_ nor parent_ is rendered, so both stay minimal here.
         **/
        TEST_CASE("interpreter2-localenv-render", "[printable][interpreter2]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(),
                                     "interpreter2-localenv-render"));

                auto check_env = [&rh, &log]
                    (const char * label, std::uint32_t n_args,
                     std::uint32_t margin, const char * expect_pretty)
                {
                    ArenaFixture fx(std::string(label) + "."
                                    + std::to_string(margin));
                    auto mm = fx.mm();

                    DLocalSymtab * symtab = DLocalSymtab::_make_empty(mm, nullptr,
                                                                     8 /*nv*/,
                                                                     4 /*nt*/);
                    DArray * args = DArray::_empty(mm, 8 /*cap*/);

                    for (std::uint32_t i = 0; i < n_args; ++i)
                        args->push_back(mm, DInteger::box(mm, i));

                    DLocalEnv * env = DLocalEnv::_make(mm, nullptr, symtab, args);

                    obj<APrintable,DLocalEnv> env_pr(env);
                    std::string pretty = render_pretty(env_pr, margin);

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                };

                check_env("empty", 0, 200,
                          "<DLocalEnv :n_args 0>");

                check_env("three", 3, 200,
                          "<DLocalEnv :n_args 3>");

                /* margin 16: the struct breaks, but the field's VALUE still
                 * fits beside its tag.  Both protocols indent the field by 2.
                 */
                check_env("three", 3, 16,
                          "<DLocalEnv\n"
                          "  :n_args 3>");

                /* margin 8: narrow enough to push the VALUE off the tag's
                 * line too -- and there the two protocols differ, legacy
                 * indenting by indent_width (2) and ppsink by
                 * tag_value_offset (1).  The divergence documented throughout
                 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md;
                 * with one scalar field it cannot compound, so this is the
                 * smallest instance of it in the tree.
                 */
                check_env("three", 3, 8,
                          "<DLocalEnv\n"
                          "  :n_args\n"
                          "   3>");
            }
        }

        /** The five independent vsm frames, converted as one batch: they
         *  share a shape (`:cont` plus at most one scalar) and share the one
         *  question worth asking, so a single table answers it five times.
         *
         *  THE QUESTION, now answered: `:cont` is a VsmInstr, which has
         *  NEITHER a Prettifier<> NOR a ppdetail<>.  Both protocols therefore
         *  reach it through the leaf fallback to operator<<, which prints the
         *  opcode name.  That is the same silent path on which ParserResult
         *  and DSchematikaParser* turned out to render a DIFFERENT struct on
         *  the ppsink side.  Measured here, not assumed: the two agree
         *  exactly -- `:cont def_cont` on both -- so no Prettifier<VsmInstr>
         *  is needed.  The margin-12 rows are what pin that, since a flat
         *  render alone would not distinguish "agrees" from "both happen to
         *  fit".
         *
         *  The ONLY difference anywhere in this table is the documented
         *  indent divergence: a value pushed onto its own line is indented by
         *  legacy's indent_width (2) and ppsink's tag_value_offset (1).
         *
         *  None of the five renders its parent/stack link, so `no_parent` is
         *  fine throughout, and none renders the expression it continues, so
         *  those are nullptr -- no constructor touches them (verified by
         *  reading all five make() bodies, which only forward and placement-new).
         *
         *  `:i_arg -1` is not a sentinel chosen for the test: it is
         *  DVsmEvalArgsFrame's initial value (DVsmEvalArgsFrame.hpp:68),
         *  incremented before each argument.
         **/
        TEST_CASE("interpreter2-vsmframe-render", "[printable][interpreter2]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(),
                                     "interpreter2-vsmframe-render"));

                auto check = [&rh, &log]
                    (const char * label, auto pr, std::uint32_t margin, const char * expect_pretty)
                {
                    std::string pretty = render_pretty(pr, margin);

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                };

                ArenaFixture fx("vsmframe");
                auto mm = fx.mm();
                obj<AGCObject> no_parent;

                obj<APrintable,DVsmDefContFrame> defcont
                    (DVsmDefContFrame::make(mm, no_parent,
                                            VsmInstr::c_def_cont, nullptr));
                obj<APrintable,DVsmIfElseContFrame> ifelsecont
                    (DVsmIfElseContFrame::make(mm, no_parent,
                                               VsmInstr::c_ifelse_cont, nullptr));
                obj<APrintable,DVsmSeqContFrame> seqcont
                    (DVsmSeqContFrame::make(mm, no_parent,
                                            VsmInstr::c_seq_cont, nullptr,
                                            7 /*i_seq*/));

                DArray * args = DArray::_empty(mm, 8);
                for (int i = 0; i < 2; ++i)
                    args->push_back(mm, DInteger::box(mm, i));

                DVsmApplyFrame * af
                    = DVsmApplyFrame::make(mm, no_parent,
                                           VsmInstr::c_apply, args);

                obj<APrintable,DVsmApplyFrame> apply(af);
                obj<APrintable,DVsmEvalArgsFrame> evalargs
                    (DVsmEvalArgsFrame::make(mm, af,
                                             VsmInstr::c_evalargs, nullptr));

                /* --- flat, margin 200 --- */

                check("defcont", defcont, 200,
                      "<DVsmDefContFrame :cont def_cont>");
                check("ifelsecont", ifelsecont, 200,
                      "<DVsmIfElseContFrame :cont ifelse_cont>");
                check("seqcont", seqcont, 200,
                      "<DVsmSeqContFrame :cont seq_cont :i_seq 7>");
                check("apply", apply, 200,
                      "<DVsmApplyFrame :cont apply :n_args 2>");
                check("evalargs", evalargs, 200,
                      "<DVsmEvalArgsFrame :cont evalargs :i_arg -1>");

                /* --- margin 12: the :cont VALUE goes to its own line, which
                 * is what actually exercises the VsmInstr leaf fallback under
                 * a break.  The second field still fits beside its tag.
                 */

                check("defcont", defcont, 12,
                      "<DVsmDefContFrame\n"
                      "  :cont\n"
                      "   def_cont>");
                check("ifelsecont", ifelsecont, 12,
                      "<DVsmIfElseContFrame\n"
                      "  :cont\n"
                      "   ifelse_cont>");
                check("seqcont", seqcont, 12,
                      "<DVsmSeqContFrame\n"
                      "  :cont\n"
                      "   seq_cont\n"
                      "  :i_seq 7>");
                check("apply", apply, 12,
                      "<DVsmApplyFrame\n"
                      "  :cont\n"
                      "   apply\n"
                      "  :n_args 2>");
                check("evalargs", evalargs, 12,
                      "<DVsmEvalArgsFrame\n"
                      "  :cont\n"
                      "   evalargs\n"
                      "  :i_arg -1>");

                /* --- margin 8: BOTH fields' values break.  Only the two-field
                 * frames render differently from margin 12, so only they are
                 * pinned here.
                 */

                check("seqcont", seqcont, 8,
                      "<DVsmSeqContFrame\n"
                      "  :cont\n"
                      "   seq_cont\n"
                      "  :i_seq\n"
                      "   7>");
                check("evalargs", evalargs, 8,
                      "<DVsmEvalArgsFrame\n"
                      "  :cont\n"
                      "   evalargs\n"
                      "  :i_arg\n"
                      "   -1>");
            }
        }

        /** DClosure -- two optional children, and the last printer in the
         *  subsystem with a facet child.
         *
         *  Legacy uses PER-FIELD present flags (unlike DLambdaExpr, whose
         *  branch is all-or-nothing), so all four combinations are distinct
         *  renderings and all four are pinned.
         *
         *  Both children are reached by DIRECT construction of
         *  obj<APrintable,T>, the shape that aborts on use in
         *  .xo-backlog/xo-reader2/issues/01-ssm-printer-null-children.md.  It
         *  is safe HERE, and only here, because the emptiness is tested before
         *  the field is emitted -- which is why DClosure is absent from that
         *  ticket.  The `closure.neither` case is what proves it: a DClosure
         *  with two null children renders `<DClosure>` rather than crashing.
         *
         *  The lambda is deliberately built with a null name and no body, so
         *  it renders as a bare `<LambdaExpr>`.  DLambdaExpr's own rendering
         *  is pinned in xo-expression2; repeating it here would couple this
         *  table to that one for no gain.
         **/
        TEST_CASE("interpreter2-closure-render", "[printable][interpreter2]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(),
                                     "interpreter2-closure-render"));

                auto check = [&rh, &log]
                    (const char * label, auto pr, std::uint32_t margin, const char * expect_pretty)
                {
                    std::string pretty = render_pretty(pr, margin);

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                };

                ArenaFixture fx("closure");
                auto mm = fx.mm();

                DLocalSymtab * symtab = DLocalSymtab::_make_empty(mm, nullptr, 2, 0);
                DArray * args = DArray::_empty(mm, 4);
                args->push_back(mm, DInteger::box(mm, 1));

                DLocalEnv * env = DLocalEnv::_make(mm, nullptr, symtab, args);

                DLambdaExpr * lm
                    = DLambdaExpr::_make(mm,
                                         TypeRef(TypeRef::type_var::from_chars("t:1"),
                                                 obj<xo::scm::AType>()),
                                         nullptr /*name*/,
                                         symtab,
                                         obj<xo::scm::AExpression>());

                obj<APrintable,DClosure> both(DClosure::make(mm, lm, env));
                obj<APrintable,DClosure> no_env(DClosure::make(mm, lm, nullptr));
                obj<APrintable,DClosure> no_lambda(DClosure::make(mm, nullptr, env));
                obj<APrintable,DClosure> neither(DClosure::make(mm, nullptr, nullptr));

                check("both", both, 200,
                      "<DClosure :lambda <LambdaExpr> :env <DLocalEnv :n_args 1>>");
                check("no-env", no_env, 200,
                      "<DClosure :lambda <LambdaExpr>>");
                check("no-lambda", no_lambda, 200,
                      "<DClosure :env <DLocalEnv :n_args 1>>");
                check("neither", neither, 200,
                      "<DClosure>");

                /* margin 24: the nested DLocalEnv breaks too, so the indent
                 * divergence COMPOUNDS -- 4 vs 3 at the :env value, then 6 vs
                 * 4 at the nested :n_args.  The two-level version of what
                 * interpreter2-localenv-render pins at one level.
                 */
                check("both", both, 24,
                      "<DClosure\n"
                      "  :lambda <LambdaExpr>\n"
                      "  :env\n"
                      "   <DLocalEnv\n"
                      "    :n_args 1>>");
                check("no-env", no_env, 24,
                      "<DClosure\n"
                      "  :lambda <LambdaExpr>>");
            }
        }

        /** DVsmApplyClosureFrame -- `:env` renders the environment, not its
         *  address.
         *
         *  It used to print the address.  `local_env_` is a raw DLocalEnv*,
         *  and passing it straight to field() sent it to the leaf fallback
         *  (operator<< on a pointer): `:env 0x7879d025b098`, and `:env 0` for
         *  null.  BOTH protocols agreed on that, so the phase-C conversion
         *  reproduced it deliberately rather than smuggling a behaviour change
         *  into a mechanical refactor, and the expectations here were pinned
         *  with the address scrubbed to `0xADDR`.
         *
         *  Fixed 2026-08-13, after phase E removed the deprecated protocol --
         *  doing it earlier would have forced the deprecated expectations to
         *  carry the old address form until they were deleted anyway.  The
         *  field now wraps in obj<APrintable,DLocalEnv> with a present flag,
         *  exactly as DClosure does for the same type.  `scrub_addr()` went
         *  with it: nothing in this file renders an address any more.
         *  See .xo-backlog/xo-interpreter2/issues/01-applyclosureframe-env-prints-address.md
         *
         *  The null case is the reason for the present flag rather than a
         *  bare wrap: local_env_ CAN be null, and `null-env` below pins that
         *  it drops the field rather than dereferencing.  Note that is a
         *  CHANGE from `:env 0` -- an omitted field, not a rendered zero.
         **/
        TEST_CASE("interpreter2-applyclosureframe-render", "[printable][interpreter2]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(),
                                     "interpreter2-applyclosureframe-render"));

                auto check = [&rh, &log]
                    (const char * label, auto pr, std::uint32_t margin, const char * expect_pretty)
                {
                    std::string pretty = render_pretty(pr, margin);

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                };

                ArenaFixture fx("applyclosure");
                auto mm = fx.mm();
                obj<AGCObject> no_parent;

                DLocalSymtab * symtab = DLocalSymtab::_make_empty(mm, nullptr, 2, 0);
                DArray * args = DArray::_empty(mm, 4);
                args->push_back(mm, DInteger::box(mm, 1));

                DLocalEnv * env = DLocalEnv::_make(mm, nullptr, symtab, args);

                obj<APrintable,DVsmApplyClosureFrame> with_env
                    (DVsmApplyClosureFrame::make(mm, no_parent,
                                                 VsmInstr::c_apply_cont, env));
                obj<APrintable,DVsmApplyClosureFrame> null_env
                    (DVsmApplyClosureFrame::make(mm, no_parent,
                                                 VsmInstr::c_apply_cont, nullptr));

                check("env", with_env, 200,
                      "<DVsmApplyClosureFrame :cont apply_cont :env <DLocalEnv :n_args 1>>");
                check("null-env", null_env, 200,
                      "<DVsmApplyClosureFrame :cont apply_cont>");

                /* margin 24: the nested DLocalEnv breaks too, so the indent
                 * divergence compounds -- the same two-level shape as
                 * interpreter2-closure-render pins for DClosure.
                 */
                check("env", with_env, 24,
                      "<DVsmApplyClosureFrame\n"
                      "  :cont apply_cont\n"
                      "  :env\n"
                      "   <DLocalEnv\n"
                      "    :n_args 1>>");
                check("null-env", null_env, 24,
                      "<DVsmApplyClosureFrame\n"
                      "  :cont apply_cont>");
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
