/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-reader2's printers, bottom-up.
 *
 * Follows the template in xo-expression2/utest/printable_render.test.cpp --
 * see .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md for why
 * both renderings are pinned rather than only asserted equal.
 *
 * DIFFERENT FROM the expression2 file in one way that matters.  There each
 * printer got its own conversion cycle, because each taught something new.
 * Here 24 printers remain and most are two-field structs over an enum and a
 * string accessor; they are converted in BATCHES by shape, with one pinned
 * table per batch.  RC's call 2026-08-11.  The batches, in order:
 *
 *   1. flat leaves        -- no facet children (this file, s_leaf_v)
 *   2. facet-nesting      -- DProgressSsm, DDefineSsm, ... , DExpectQDictSsm
 *   3. DExpectFormalArglistSsm, ParserStack -- the only two hand-rolled
 *                            two-pass printers in the subsystem
 *   4. DSchematikaParser  -- last, it renders :stack
 *
 * The one thing this batch is really testing, uniformly: reader2's printers
 * are full of ENUMS (parenstate, quote_xst, deftypestate, fstate, seqtype),
 * none of which has a Prettifier<>.  They reach ppsink through its leaf
 * FALLBACK to operator<< -- the same silent path DVarRef's Binding field took.
 * Whether that fallback agrees with legacy for an enum is one question with
 * twelve answers, which is the argument for doing them together.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/reader2/DeftypeSsm.hpp>
#include <xo/reader2/ExpectExprSsm.hpp>
#include <xo/reader2/ExpectFormalArgSsm.hpp>
#include <xo/reader2/ExpectListTypeSsm.hpp>
#include <xo/reader2/ExpectQLiteralSsm.hpp>
#include <xo/reader2/ExpectSymbolSsm.hpp>
#include <xo/reader2/ExpectTypeSsm.hpp>
#include <xo/reader2/ParenSsm.hpp>
#include <xo/reader2/QuoteSsm.hpp>
#include <xo/reader2/SequenceSsm.hpp>
#include <xo/reader2/ToplevelSeqSsm.hpp>
#include <xo/reader2/GlobalEnv.hpp>
#include <xo/reader2/init_reader2.hpp>
#include <xo/expression2/GlobalSymtab.hpp>
#include <xo/arena/ArenaHashMapConfig.hpp>
#include <xo/gc/X1Collector.hpp>
#include <xo/gc/init_gc.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/alloc2/CollectorTypeRegistry.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/facet/FacetRegistry.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/indentlog/print/ppstr.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    using xo::mm::ArenaConfig;
    using xo::mm::AAllocator;
    using xo::mm::ACollector;
    using xo::mm::DArena;
    using xo::mm::DX1Collector;
    using xo::mm::X1CollectorConfig;
    using xo::mm::CollectorTypeRegistry;
    using xo::scm::DParenSsm;
    using xo::scm::DQuoteSsm;
    using xo::scm::DDeftypeSsm;
    using xo::scm::DExpectSymbolSsm;
    using xo::scm::DExpectListTypeSsm;
    using xo::scm::DExpectFormalArgSsm;
    using xo::scm::DExpectTypeSsm;
    using xo::scm::DExpectQLiteralSsm;
    using xo::scm::DExpectExprSsm;
    using xo::scm::DSequenceSsm;
    using xo::scm::DToplevelSeqSsm;
    using xo::scm::DGlobalEnv;
    using xo::scm::DGlobalSymtab;
    using xo::map::ArenaHashMapConfig;
    using xo::print::APrintable;
    using xo::facet::obj;
    using xo::facet::with_facet;

    static InitEvidence s_init = (InitSubsys<S_reader2_tag>::require()
                                  ^ InitSubsys<S_gc_tag>::require());

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            /** render @p x through the DEPRECATED two-pass protocol.
             *  DELETE AT PHASE E, with expect_deprecated_ and its REHEARSE.
             **/
            template <typename T>
            std::string
            render_deprecated(const T & x, std::uint32_t margin) {
                xo::print::ppconfig ppc;
                ppc.right_margin_ = margin;

                bool orig_color = xo::tag_config::tag_color_enabled;
                xo::tag_config::tag_color_enabled = false;

                xo::pp::default_style_guard plain(xo::pp::PpStyle::plain());

                std::string retval = xo::toppstr2(ppc, x);

                xo::tag_config::tag_color_enabled = orig_color;

                return retval;
            }

            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string
            render_pretty(const T & x, std::uint32_t margin) {
                return toppstr(PpConfig::scratch_plain(margin), x);
            }

            /** arena for parser-owned state machines, plus a collector for the
             *  two leaves that need expression memory.
             *
             *  Deliberately NOT reusing SchematikaParser.test.cpp's
             *  ParserFixture: that builds a whole DSchematikaParser to drive
             *  source text through.  Every printer in this batch is reachable
             *  from a bare `_make(parser_mm)`, and a fixture that cannot fail
             *  for parser reasons is a better witness for a printer test.
             **/
            struct SsmFixture {
                explicit SsmFixture(const std::string & testname)
                    : parser_arena_{ArenaConfig()
                                    .with_name("printable_render." + testname)
                                    .with_size(64 * 1024)
                                    .with_store_header_flag(true)},
                      gc_{X1CollectorConfig{
                              .name_ = "printable_render.gc." + testname,
                              .arena_config_ = ArenaConfig{
                                  .size_ = 8192,
                                  .store_header_flag_ = true},
                              .object_types_z_ = 16384,
                              .gc_trigger_v_{{4096, 4096}},
                              .debug_flag_ = false}}
                {
                    CollectorTypeRegistry::instance()
                        .install_types(with_facet<ACollector>::mkobj(&gc_));
                }

                DArena & parser_mm() { return parser_arena_; }

                /** DGlobalEnv is the one leaf needing more than an arena: it
                 *  renders symtab_->n_vars(), so it needs a real symtab.  The
                 *  hash-map superstructure lives outside GC space, hence the
                 *  parser arena as fixed_mm -- DGlobalSymtab::make's own
                 *  mm / fixed_mm split, not a testing convenience.
                 **/
                DGlobalEnv * make_global_env() {
                    auto cfg = ArenaHashMapConfig()
                        .with_name("printable_render.symtab")
                        .with_hint_max_capacity(16);

                    DGlobalSymtab * symtab
                        = DGlobalSymtab::_make(this->expr_mm(),
                                               with_facet<AAllocator>::mkobj(&parser_arena_),
                                               cfg, cfg);

                    return DGlobalEnv::_make(this->expr_mm(), symtab);
                }

                obj<AAllocator> expr_mm() { return with_facet<AAllocator>::mkobj(&gc_); }

                DArena parser_arena_;
                DX1Collector gc_;
            };
        } /*namespace*/

        /** BATCH 1: the flat leaves -- every xo-reader2 printer with no facet
         *  child.  Thirteen classes, fifteen cases (DExpectTypeSsm and
         *  DExpectExprSsm each get two, for arguments that do and do not reach
         *  the output), at margins 200 and 30.
         *
         *  Batched rather than one cycle per printer: RC's call 2026-08-11.
         *  See the file header for why, and for the batch order.
         *
         *  WHAT THIS BATCH ESTABLISHED, and the reason it is worth doing as
         *  one unit: reader2's printers are full of enums -- parenstate,
         *  quote_xst, deftypestate, fstate, seqtype -- and NONE has a
         *  Prettifier<>.  Each reaches ppsink through its leaf FALLBACK to
         *  operator<<, the same silent path DVarRef's Binding field took.  The
         *  fallback agrees with legacy EXACTLY, in every case: lparen_0,
         *  quote_0, def_0, formal_0, toplevel-interactive, toplevel-batch.
         *
         *  The only divergence anywhere in the batch is the familiar
         *  field-value column -- legacy indent+2, ppsink indent+1 -- visible
         *  on the wrapped :expect values at margin 30 and nowhere else.
         *
         *  Three of these render BARE, with no fields, and each for its own
         *  reason worth not "fixing" silently: DExpectSymbolSsm's :member
         *  field is commented out in legacy; DExpectListTypeSsm prints nothing
         *  though it holds an elt_type_; DExpectTypeSsm ignores its
         *  `corrected` constructor argument (both cases below pin that it
         *  makes no difference).
         *
         *  Expectations are OBSERVED, never predicted, and were generated from
         *  the observed bytes rather than transcribed.
         **/
        TEST_CASE("reader2-leaf-render", "[printable][reader2]")
        {
            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "reader2-leaf-render"));

                SsmFixture fx("leaves");
                DArena & mm = fx.parser_mm();

                DToplevelSeqSsm tl_i(xo::scm::exprseqtype::toplevel_interactive);
                DToplevelSeqSsm tl_b(xo::scm::exprseqtype::toplevel_batch);

                auto check = [&rh, &log](const char * label, auto * p,
                                         std::uint32_t margin,
                                         const char * expect_deprecated,
                                         const char * expect_pretty)
                {
                    auto pr = with_facet<APrintable>::mkobj(p);

                    std::string deprecated = render_deprecated(pr, margin);
                    std::string pretty = render_pretty(pr, margin);

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                    REHEARSE(rh, deprecated == std::string(expect_deprecated));
                };

                check("DParenSsm", DParenSsm::_make(mm), 200,
                      "<DParenSsm :parenstate lparen_0 :expect leftparen>",
                      "<DParenSsm :parenstate lparen_0 :expect leftparen>");

                check("DParenSsm", DParenSsm::_make(mm), 30,
                      "<DParenSsm\n"
                      "  :parenstate lparen_0\n"
                      "  :expect leftparen>",
                      "<DParenSsm\n"
                      "  :parenstate lparen_0\n"
                      "  :expect leftparen>");

                check("DQuoteSsm", DQuoteSsm::_make(mm), 200,
                      "<DQuoteSsm :quote_xst quote_0 :expect #q>",
                      "<DQuoteSsm :quote_xst quote_0 :expect #q>");

                check("DQuoteSsm", DQuoteSsm::_make(mm), 30,
                      "<DQuoteSsm\n"
                      "  :quote_xst quote_0\n"
                      "  :expect #q>",
                      "<DQuoteSsm\n"
                      "  :quote_xst quote_0\n"
                      "  :expect #q>");

                check("DDeftypeSsm", DDeftypeSsm::_make(mm), 200,
                      "<DDeftypeSsm :deftypestate def_0 :expect deftype>",
                      "<DDeftypeSsm :deftypestate def_0 :expect deftype>");

                check("DDeftypeSsm", DDeftypeSsm::_make(mm), 30,
                      "<DDeftypeSsm\n"
                      "  :deftypestate def_0\n"
                      "  :expect deftype>",
                      "<DDeftypeSsm\n"
                      "  :deftypestate def_0\n"
                      "  :expect deftype>");

                check("DExpectSymbolSsm", DExpectSymbolSsm::_make(mm), 200,
                      "<DExpectSymbolSsm>",
                      "<DExpectSymbolSsm>");

                check("DExpectSymbolSsm", DExpectSymbolSsm::_make(mm), 30,
                      "<DExpectSymbolSsm>",
                      "<DExpectSymbolSsm>");

                check("DExpectListTypeSsm", DExpectListTypeSsm::_make(mm), 200,
                      "<DExpectListTypeSsm>",
                      "<DExpectListTypeSsm>");

                check("DExpectListTypeSsm", DExpectListTypeSsm::_make(mm), 30,
                      "<DExpectListTypeSsm>",
                      "<DExpectListTypeSsm>");

                check("DExpectFormalArgSsm", DExpectFormalArgSsm::_make(mm), 200,
                      "<DExpectFormalArgSsm :fstate formal_0 :expect formal-name>",
                      "<DExpectFormalArgSsm :fstate formal_0 :expect formal-name>");

                check("DExpectFormalArgSsm", DExpectFormalArgSsm::_make(mm), 30,
                      "<DExpectFormalArgSsm\n"
                      "  :fstate formal_0\n"
                      "  :expect formal-name>",
                      "<DExpectFormalArgSsm\n"
                      "  :fstate formal_0\n"
                      "  :expect formal-name>");

                check("DExpectTypeSsm.f", DExpectTypeSsm::_make(mm, false), 200,
                      "<DExpectTypeSsm>",
                      "<DExpectTypeSsm>");

                check("DExpectTypeSsm.f", DExpectTypeSsm::_make(mm, false), 30,
                      "<DExpectTypeSsm>",
                      "<DExpectTypeSsm>");

                check("DExpectTypeSsm.t", DExpectTypeSsm::_make(mm, true), 200,
                      "<DExpectTypeSsm>",
                      "<DExpectTypeSsm>");

                check("DExpectTypeSsm.t", DExpectTypeSsm::_make(mm, true), 30,
                      "<DExpectTypeSsm>",
                      "<DExpectTypeSsm>");

                check("DExpectQLiteralSsm", DExpectQLiteralSsm::_make(mm, false, false), 200,
                      "<DExpectQLiteralSsm :expect leftparen|leftbracket|leftbrace|string|f64|i64|bool>",
                      "<DExpectQLiteralSsm :expect leftparen|leftbracket|leftbrace|string|f64|i64|bool>");

                check("DExpectQLiteralSsm", DExpectQLiteralSsm::_make(mm, false, false), 30,
                      "<DExpectQLiteralSsm\n"
                      "  :expect\n"
                      "    leftparen|leftbracket|leftbrace|string|f64|i64|bool>",
                      "<DExpectQLiteralSsm\n"
                      "  :expect\n"
                      "   leftparen|leftbracket|leftbrace|string|f64|i64|bool>");

                check("DExpectExprSsm.fff", DExpectExprSsm::_make(mm, false, false, false), 200,
                      "<DExpectExprSsm :allow_defs 0 :cxl_on_rightbrace 0 :cxl_on_rightparen 0 :expect if|lambda|lparen|lbrace|literal|var>",
                      "<DExpectExprSsm :allow_defs 0 :cxl_on_rightbrace 0 :cxl_on_rightparen 0 :expect if|lambda|lparen|lbrace|literal|var>");

                check("DExpectExprSsm.fff", DExpectExprSsm::_make(mm, false, false, false), 30,
                      "<DExpectExprSsm\n"
                      "  :allow_defs 0\n"
                      "  :cxl_on_rightbrace 0\n"
                      "  :cxl_on_rightparen 0\n"
                      "  :expect\n"
                      "    if|lambda|lparen|lbrace|literal|var>",
                      "<DExpectExprSsm\n"
                      "  :allow_defs 0\n"
                      "  :cxl_on_rightbrace 0\n"
                      "  :cxl_on_rightparen 0\n"
                      "  :expect\n"
                      "   if|lambda|lparen|lbrace|literal|var>");

                check("DExpectExprSsm.ttt", DExpectExprSsm::_make(mm, true, true, true), 200,
                      "<DExpectExprSsm :allow_defs 1 :cxl_on_rightbrace 1 :cxl_on_rightparen 1 :expect def|if|lambda|lparen|lbrace|literal|var>",
                      "<DExpectExprSsm :allow_defs 1 :cxl_on_rightbrace 1 :cxl_on_rightparen 1 :expect def|if|lambda|lparen|lbrace|literal|var>");

                check("DExpectExprSsm.ttt", DExpectExprSsm::_make(mm, true, true, true), 30,
                      "<DExpectExprSsm\n"
                      "  :allow_defs 1\n"
                      "  :cxl_on_rightbrace 1\n"
                      "  :cxl_on_rightparen 1\n"
                      "  :expect\n"
                      "    def|if|lambda|lparen|lbrace|literal|var>",
                      "<DExpectExprSsm\n"
                      "  :allow_defs 1\n"
                      "  :cxl_on_rightbrace 1\n"
                      "  :cxl_on_rightparen 1\n"
                      "  :expect\n"
                      "   def|if|lambda|lparen|lbrace|literal|var>");

                check("DSequenceSsm", DSequenceSsm::_make(mm, fx.expr_mm()), 200,
                      "<DSequenceSsm :seq_expr.size 0 :expect expr|semicolon|rightbrace>",
                      "<DSequenceSsm :seq_expr.size 0 :expect expr|semicolon|rightbrace>");

                check("DSequenceSsm", DSequenceSsm::_make(mm, fx.expr_mm()), 30,
                      "<DSequenceSsm\n"
                      "  :seq_expr.size 0\n"
                      "  :expect\n"
                      "    expr|semicolon|rightbrace>",
                      "<DSequenceSsm\n"
                      "  :seq_expr.size 0\n"
                      "  :expect\n"
                      "   expr|semicolon|rightbrace>");

                check("DToplevelSeqSsm.i", &tl_i, 200,
                      "<DToplevelSeqSsm :seqtype toplevel-interactive>",
                      "<DToplevelSeqSsm :seqtype toplevel-interactive>");

                check("DToplevelSeqSsm.i", &tl_i, 30,
                      "<DToplevelSeqSsm\n"
                      "  :seqtype\n"
                      "    toplevel-interactive>",
                      "<DToplevelSeqSsm\n"
                      "  :seqtype\n"
                      "   toplevel-interactive>");

                check("DToplevelSeqSsm.b", &tl_b, 200,
                      "<DToplevelSeqSsm :seqtype toplevel-batch>",
                      "<DToplevelSeqSsm :seqtype toplevel-batch>");

                check("DToplevelSeqSsm.b", &tl_b, 30,
                      "<DToplevelSeqSsm\n"
                      "  :seqtype toplevel-batch>",
                      "<DToplevelSeqSsm\n"
                      "  :seqtype toplevel-batch>");

                check("DGlobalEnv", fx.make_global_env(), 200,
                      "<DGlobalEnv :n_vars 0>",
                      "<DGlobalEnv :n_vars 0>");

                check("DGlobalEnv", fx.make_global_env(), 30,
                      "<DGlobalEnv :n_vars 0>",
                      "<DGlobalEnv :n_vars 0>");
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
