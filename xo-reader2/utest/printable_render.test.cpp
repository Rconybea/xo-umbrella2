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
#include <xo/reader2/ApplySsm.hpp>
#include <xo/reader2/DefineSsm.hpp>
#include <xo/reader2/IfElseSsm.hpp>
#include <xo/reader2/LambdaSsm.hpp>
#include <xo/reader2/ProgressSsm.hpp>
#include <xo/reader2/ExpectQListSsm.hpp>
#include <xo/reader2/ExpectQArraySsm.hpp>
#include <xo/reader2/ExpectQDictSsm.hpp>
#include <xo/reader2/ParserResult.hpp>
#include <xo/reader2/SchematikaParser.hpp>
#include <xo/reader2/init_reader2.hpp>
#include <xo/expression2/DefineExpr.hpp>
#include <xo/expression2/IfElseExpr.hpp>
#include <xo/expression2/Constant.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Dictionary.hpp>
#include <xo/stringtable2/String.hpp>
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
    using xo::mm::AGCObject;
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
    using xo::scm::DSchematikaParser;
    using xo::scm::ParserConfig;
    using xo::mm::dp;
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

            /* THREE process-wide counters leak into these renderings, exactly
             * as they do in xo-expression2's phase-C table, and for the same
             * reasons.  Copied verbatim from
             * xo-expression2/utest/printable_render.test.cpp rather than
             * shared: they are test scaffolding for observed output, not API.
             */
            /** replace the digits of ":id <n>" with "N".  See the identical
             *  helper in xo-procedure2/utest/printable_render.test.cpp: TypeId
             *  is a process-wide counter handed out in reflection order, so it
             *  moves when an unrelated test reflects a new type first.
             *
             *  This scrubs the NESTED TypeDescr's :id.  TypeRef's own :id is a
             *  type-variable NAME, is quoted, and stays pinned exactly.
             **/
            std::string scrub_type_id(std::string s) {
                const std::string key = ":id ";

                for (std::size_t i = s.find(key); i != std::string::npos; i = s.find(key, i+1)) {
                    std::size_t b = i + key.size();
                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b)
                        s.replace(b, e - b, "N");
                }

                return s;
            }

            /** index of the digits belonging to the ".tseq" at/after @p from,
             *  or npos.  @p n_digit receives their count.
             *
             *  NB the separator is whitespace, not necessarily a SPACE: at a
             *  narrow margin the value breaks onto its own line, so ".tseq" is
             *  followed by "\n" plus indent.  Keying on ".tseq " silently
             *  scrubbed nothing in exactly those cases -- caught because the
             *  broken-layout expectations then failed.
             **/
            std::size_t find_tseq_digits(const std::string & s, std::size_t from,
                                         std::size_t * n_digit) {
                const std::string key = ".tseq";

                for (std::size_t i = s.find(key, from); i != std::string::npos;
                     i = s.find(key, i+1))
                {
                    std::size_t b = i + key.size();

                    while (b < s.size() && ::isspace((unsigned char)s[b]))
                        ++b;

                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b) {
                        *n_digit = e - b;
                        return b;
                    }
                }

                return std::string::npos;
            }

            /** replace the digits in a quoted GENERATED type-variable name:
             *  :id "if:12"  ->  :id "if:N".
             *
             *  A third counter, distinct from TypeDescr's :id (scrub_type_id)
             *  and from typeseq (scrub_tseq).  DIfElseExpr::_make_empty() builds
             *  its TypeRef through TypeRef::generate_unique(), which draws on a
             *  process-wide counter -- so these move with how many TypeRefs the
             *  run happened to make before this one, including from unrelated
             *  tests.  The PREFIX is kept: "if:" is a property of the printer
             *  under test, the number is not.
             **/
            std::string scrub_typevar(std::string s) {
                const std::string key = ":id \"";

                for (std::size_t i = s.find(key); i != std::string::npos; i = s.find(key, i+1)) {
                    std::size_t b = i + key.size();

                    /* skip the prefix, up to and including its ':' */
                    while (b < s.size() && s[b] != ':' && s[b] != '"')
                        ++b;

                    if (b >= s.size() || s[b] != ':')
                        continue;   /* no prefix separator -- not a generated name */

                    ++b;

                    std::size_t e = b;

                    while (e < s.size() && ::isdigit((unsigned char)s[e]))
                        ++e;

                    if (e > b)
                        s.replace(b, e - b, "N");
                }

                return s;
            }

            /** replace each ".tseq" value with "N".
             *
             *  Same reasoning as scrub_type_id, different counter: a typeseq is
             *  handed out by xo::reflect::typeseq::id<T>() on first use, and in
             *  practice that is subsystem registration order
             *  (SetupObject2::register_facets etc).  Stable today -- DInteger
             *  is 9 and DFloat 10 on every run -- but it is registration order,
             *  not a property of DConstant, so pinning the digits would make an
             *  unrelated object2 registration break this test.
             *
             *  What the digits actually SAY is checked separately, in
             *  DConstant-tseq-fields below.
             **/
            std::string scrub_tseq(std::string s) {
                std::size_t n_digit = 0;

                for (std::size_t i = find_tseq_digits(s, 0, &n_digit);
                     i != std::string::npos;
                     i = find_tseq_digits(s, i+1, &n_digit))
                {
                    s.replace(i, n_digit, "N");
                }

                return s;
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

            /** BATCH 2 needs a real parser and batch 1 did not.  Four of
             *  batch 2's printers hold children that only exist mid-parse,
             *  and for three of them the EMPTY case is not renderable at all
             *  -- DExpectQList/QArraySsm throw, DExpectQDictSsm aborts.  So
             *  stepping a parser one token at a time and reading top_ssm()
             *  between tokens is the only way to pin them.  Batch 3's
             *  ParserStack will want the same machinery.
             *
             *  Structure copied from SchematikaParser.test.cpp's
             *  ParserFixture rather than bolted onto SsmFixture: the
             *  collector must be allocated FROM the aux arena and the parser
             *  registered as a GC root.  A version that skipped both
             *  segfaulted before the first token.
             **/
            struct ParseFixture {
                explicit ParseFixture(const std::string & testname) {
                    aux_arena_ = std::move(DArena(ArenaConfig()
                                                  .with_name(testname)
                                                  .with_size(64 * 1024)
                                                  .with_store_header_flag(true)));
                    obj<AAllocator,DArena> aux_mm(&aux_arena_);

                    X1CollectorConfig x1_config
                        = (X1CollectorConfig()
                           .with_name("printable_render.gc." + testname)
                           .with_size(32 * 1024)
                           .with_debug_flag(false)
                           .with_sanitize_flag(true));

                    dp<DX1Collector> expr_x1_dp
                        = dp<DX1Collector>::make(aux_mm, x1_config);

                    this->expr_mm_
                        = obj<AAllocator,DX1Collector>(expr_x1_dp.release());

                    obj<ACollector> gc = expr_mm_.to_facet<ACollector>();

                    CollectorTypeRegistry::instance().install_types(gc);

                    ParserConfig cfg;
                    cfg.parser_arena_config_.size_ = 16 * 1024;
                    cfg.symtab_var_config_.hint_max_capacity_ = 128;
                    cfg.symtab_types_config_.hint_max_capacity_ = 64;
                    cfg.max_stringtable_capacity_ = 512;
                    cfg.debug_flag_ = false;

                    this->parser_
                        = DSchematikaParser::make(aux_mm, cfg, expr_mm_, aux_mm);
                    this->parser_gco_ = parser_;

                    gc.add_gc_root_poly(&parser_gco_);
                }

                ParseFixture(const ParseFixture &) = delete;

                ~ParseFixture() {
                    expr_mm_._drop();
                    parser_._drop();
                }

                DArena aux_arena_;
                obj<AAllocator> expr_mm_;
                obj<AGCObject,DSchematikaParser> parser_;
                obj<AGCObject> parser_gco_;
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

        /** BATCH 2: the facet-nesting printers -- those whose fields include
         *  another printer reached through APrintable.
         *
         *  SEVEN of the nine planned, not nine.  DExpectQListSsm and
         *  DExpectQArraySsm moved OUT of this batch on 2026-08-11, after
         *  observation rather than by plan: neither can be pinned yet.
         *  Their empty state is not renderable on the legacy protocol at all
         *  (`variant<APrintable,AGCObject>` on a null child throws), and
         *  their populated state never reaches the top of the parser stack --
         *  each element of a quoted list pushes its own DExpectQLiteralSsm on
         *  top of them, so top_ssm() shows the element, never the container.
         *  They are visible only inside a whole-parser rendering, at :[1] of
         *  :stack, which on the ppsink side still stops at STUB:ParserStack.
         *  So they convert AFTER ParserStack and DSchematikaParser, where the
         *  observation window exists.  See the ticket.
         *
         *  WHAT THIS BATCH ESTABLISHED:
         *
         *  1. field()'s `present` flag absorbs every optional in the batch.
         *     DProgressSsm has THREE (:lhs :op :rhs), DApplySsm and
         *     DLambdaSsm one each.  DLambdaSsm's legacy if/else COLLAPSED --
         *     both arms rendered :lmstate and :expect and only :body differed
         *     -- which is the DExpectFormalArgSsm shape, not DLambdaExpr's.
         *
         *  2. ParserResult's switch does NOT collapse, and is the one place
         *     in the batch where a branch had to stay.  Its three arms have
         *     three different ARITIES, and folding them onto present-flags
         *     would mean calling variant<APrintable,AExpression> on the none
         *     and error paths, where result_expr_ is null and that throws.
         *
         *  3. ParserResult also needed a Prettifier<> added, not just a
         *     pretty() body: it is not a facet type, so before this it fell
         *     through ppsink's leaf fallback to operator<< and rendered
         *     ParserResult::print(ostream&) -- a DIFFERENT struct (:expr and
         *     :src_fn always present, :error quoted, never wrapping).  Its
         *     phase-B stub was dead code that nothing ever called.  Two more
         *     printers are in that position: ParserStack* and
         *     DSchematikaParser*, both still ppdetail-only.
         *
         *  4. Two printers have states in which they CANNOT be printed, both
         *     pre-existing and both reproduced rather than fixed.
         *     DDefineSsm::get_expect_str() asserts for defstate_ == def_0,
         *     which is exactly what _make() leaves behind -- hence the
         *     parser-driven cases below.  DExpectQDictSsm builds its :dict
         *     handle by DIRECT construction rather than a registry lookup, so
         *     a null dict_ yields an empty obj<> that ABORTS rather than
         *     throwing.  Tickets for both.
         *
         *  The only divergence between the two protocols anywhere in the
         *  batch is inherited, never introduced here:
         *    - the familiar field-value column, legacy indent+2 vs ppsink
         *      indent+1, compounding once per nesting level; and
         *    - DDictionary's empty/closing padding ({ } vs {}), already
         *      pinned deliberately at xo-object2/utest/printable_render.test.cpp
         *      as Testcase_Dict(80, {}, "{ }", "{}").
         *
         *  Expectations are OBSERVED, never predicted, and were generated
         *  from the observed bytes rather than transcribed.
         **/
        TEST_CASE("reader2-nesting-render", "[printable][reader2]")
        {
            using xo::scm::DApplySsm;
            using xo::scm::DIfElseSsm;
            using xo::scm::DLambdaSsm;
            using xo::scm::DProgressSsm;
            using xo::scm::ParserResult;
            using xo::scm::DIfElseExpr;
            using xo::scm::DConstant;
            using xo::scm::DInteger;
            using xo::scm::DString;
            using xo::scm::AExpression;
            using xo::scm::optype;

            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "reader2-nesting-render"));

                SsmFixture fx("nesting");
                DArena & mm = fx.parser_mm();
                obj<AAllocator> emm = fx.expr_mm();

                auto konst = [&emm](int i) {
                    return obj<AExpression>(
                        DConstant::make(emm,
                                        DInteger::box<AGCObject>(emm, i)));
                };

                auto check = [&rh, &log](const char * label, auto * p,
                                         std::uint32_t margin,
                                         const char * expect_deprecated,
                                         const char * expect_pretty)
                {
                    auto pr = with_facet<APrintable>::mkobj(p);

                    std::string deprecated
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_deprecated(pr, margin))));
                    std::string pretty
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_pretty(pr, margin))));

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("deprecated", deprecated),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                    REHEARSE(rh, deprecated == std::string(expect_deprecated));
                };

                /** ParserResult is not a facet type: no with_facet<>. **/
                auto check_pr = [&rh, &log](const char * label,
                                            const ParserResult & x,
                                            std::uint32_t margin,
                                            const char * expect_deprecated,
                                            const char * expect_pretty)
                {
                    std::string deprecated
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_deprecated(x, margin))));
                    std::string pretty
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_pretty(x, margin))));

                    log && log(xtag("label", label), xtag("margin", margin),
                               xtag("deprecated", deprecated),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                    REHEARSE(rh, deprecated == std::string(expect_deprecated));
                };

                ParserResult r_none;
                ParserResult r_expr = ParserResult::expression("some_ssm", konst(5));
                ParserResult r_err
                    = ParserResult::error("some_ssm",
                                          DString::from_view(
                                              emm, std::string_view("bad juju")));

                /* :fn_expr absent -- the present-flag path. */
                check("Apply.null", DApplySsm::_make(mm, obj<AExpression>()), 200,
                      "<DApplySsm :applystate apply_0 :expect expr>",
                      "<DApplySsm :applystate apply_0 :expect expr>");

                check("Apply.null", DApplySsm::_make(mm, obj<AExpression>()), 30,
                      "<DApplySsm\n"
                      "  :applystate apply_0\n"
                      "  :expect expr>",
                      "<DApplySsm\n"
                      "  :applystate apply_0\n"
                      "  :expect expr>");

                /* :fn_expr present.  Note _make advances applystate to
                 * apply_1, so the two cases differ in the enum too.
                 */
                check("Apply.fn", DApplySsm::_make(mm, konst(7)), 200,
                      "<DApplySsm :applystate apply_1 :expect lparen :fn_expr <DConstant :value_.tseq N :value.tseq N :value 7>>",
                      "<DApplySsm :applystate apply_1 :expect lparen :fn_expr <DConstant :value_.tseq N :value.tseq N :value 7>>");

                check("Apply.fn", DApplySsm::_make(mm, konst(7)), 30,
                      "<DApplySsm\n"
                      "  :applystate apply_1\n"
                      "  :expect lparen\n"
                      "  :fn_expr\n"
                      "    <DConstant\n"
                      "      :value_.tseq N\n"
                      "      :value.tseq N\n"
                      "      :value 7>>",
                      "<DApplySsm\n"
                      "  :applystate apply_1\n"
                      "  :expect lparen\n"
                      "  :fn_expr\n"
                      "   <DConstant\n"
                      "    :value_.tseq N\n"
                      "    :value.tseq N\n"
                      "    :value 7>>");

                /* the one batch-2 printer with NO :expect field. */
                check("IfElse", DIfElseSsm::_make(mm, DIfElseExpr::_make_empty(emm)), 200,
                      "<DIfElseSsm :ifstate if_0 :if_expr <DIfElseExpr :typeref <TypeRef :id \"if:N\" :td null>>>",
                      "<DIfElseSsm :ifstate if_0 :if_expr <DIfElseExpr :typeref <TypeRef :id \"if:N\" :td null>>>");

                check("IfElse", DIfElseSsm::_make(mm, DIfElseExpr::_make_empty(emm)), 30,
                      "<DIfElseSsm\n"
                      "  :ifstate if_0\n"
                      "  :if_expr\n"
                      "    <DIfElseExpr\n"
                      "      :typeref\n"
                      "        <TypeRef\n"
                      "          :id \"if:N\"\n"
                      "          :td null>>>",
                      "<DIfElseSsm\n"
                      "  :ifstate if_0\n"
                      "  :if_expr\n"
                      "   <DIfElseExpr\n"
                      "    :typeref\n"
                      "     <TypeRef\n"
                      "      :id \"if:N\"\n"
                      "      :td null>>>");

                /* :body absent.  The present FLAG is pinned -- forcing it
                 * true is caught by this case.  What is NOT pinned, and
                 * cannot be, is the rendering of a non-null body: body_ is
                 * assigned only immediately before this ssm pops, so it is
                 * null at every token boundary.
                 */
                check("Lambda.bare", DLambdaSsm::_make(mm), 200,
                      "<DLambdaSsm :lmstate lm_0 :expect lambda>",
                      "<DLambdaSsm :lmstate lm_0 :expect lambda>");

                check("Lambda.bare", DLambdaSsm::_make(mm), 30,
                      "<DLambdaSsm\n"
                      "  :lmstate lm_0\n"
                      "  :expect lambda>",
                      "<DLambdaSsm\n"
                      "  :lmstate lm_0\n"
                      "  :expect lambda>");

                /* all three optionals absent: only :expect survives. */
                check("Progress.bare", DProgressSsm::_make(mm, obj<AExpression>(), optype::invalid), 200,
                      "<DProgressSsm :expect expr1|leftparen>",
                      "<DProgressSsm :expect expr1|leftparen>");

                check("Progress.bare", DProgressSsm::_make(mm, obj<AExpression>(), optype::invalid), 30,
                      "<DProgressSsm\n"
                      "  :expect expr1|leftparen>",
                      "<DProgressSsm\n"
                      "  :expect expr1|leftparen>");

                /* :lhs and :op present, :rhs absent -- rhs_ is set during
                 * parsing and _make has no argument for it.
                 */
                check("Progress.lhs", DProgressSsm::_make(mm, konst(3), optype::op_add), 200,
                      "<DProgressSsm :lhs <DConstant :value_.tseq N :value.tseq N :value 3> :op op+ :expect expr2|leftparen>",
                      "<DProgressSsm :lhs <DConstant :value_.tseq N :value.tseq N :value 3> :op op+ :expect expr2|leftparen>");

                check("Progress.lhs", DProgressSsm::_make(mm, konst(3), optype::op_add), 30,
                      "<DProgressSsm\n"
                      "  :lhs\n"
                      "    <DConstant\n"
                      "      :value_.tseq N\n"
                      "      :value.tseq N\n"
                      "      :value 3>\n"
                      "  :op op+\n"
                      "  :expect expr2|leftparen>",
                      "<DProgressSsm\n"
                      "  :lhs\n"
                      "   <DConstant\n"
                      "    :value_.tseq N\n"
                      "    :value.tseq N\n"
                      "    :value 3>\n"
                      "  :op op+\n"
                      "  :expect expr2|leftparen>");

                /* ParserResult arm 1 of 3: :type alone. */
                check_pr("Result.none", r_none, 200,
                      "<ParserResult :type none>",
                      "<ParserResult :type none>");

                check_pr("Result.none", r_none, 30,
                      "<ParserResult :type none>",
                      "<ParserResult :type none>");

                /* arm 2: :type :expr. */
                check_pr("Result.expr", r_expr, 200,
                      "<ParserResult :type expression :expr <DConstant :value_.tseq N :value.tseq N :value 5>>",
                      "<ParserResult :type expression :expr <DConstant :value_.tseq N :value.tseq N :value 5>>");

                check_pr("Result.expr", r_expr, 30,
                      "<ParserResult\n"
                         "  :type expression\n"
                         "  :expr\n"
                         "    <DConstant\n"
                         "      :value_.tseq N\n"
                         "      :value.tseq N\n"
                         "      :value 5>>",
                      "<ParserResult\n"
                         "  :type expression\n"
                         "  :expr\n"
                         "   <DConstant\n"
                         "    :value_.tseq N\n"
                         "    :value.tseq N\n"
                         "    :value 5>>");

                /* arm 3: :type :src_fn :error.  Three arities, which is why
                 * the switch could not collapse.
                 */
                check_pr("Result.err", r_err, 200,
                      "<ParserResult :type error :src_fn some_ssm :error bad juju>",
                      "<ParserResult :type error :src_fn some_ssm :error bad juju>");

                check_pr("Result.err", r_err, 30,
                      "<ParserResult\n"
                         "  :type error\n"
                         "  :src_fn some_ssm\n"
                         "  :error bad juju>",
                      "<ParserResult\n"
                         "  :type error\n"
                         "  :src_fn some_ssm\n"
                         "  :error bad juju>");

            }
        }

        /** BATCH 2, part 2: the cases only a RUNNING PARSER can reach.
         *
         *  DDefineSsm and DExpectQDictSsm cannot be pinned from a bare
         *  _make(): DDefineSsm::get_expect_str() asserts for the def_0 state
         *  _make leaves behind, and DExpectQDictSsm's dict_ is null until the
         *  opening brace arrives (and a null dict_ ABORTS, it does not throw).
         *  So both are driven from real token streams and read off the top of
         *  the parser stack between tokens.
         *
         *  The token index is part of the case: it names the SSM STATE being
         *  pinned, and those states are what the printers' fields report.
         **/
        TEST_CASE("reader2-parser-render", "[printable][reader2]")
        {
            using xo::scm::Token;

            REQUIRE(s_init.evidence());

            UtestRehearser rh;

            /*  def foo : f64 = 3.5 ;
             *      ^     ^
             *      tk1   tk3      -- DDefineSsm at def_2 and def_4
             */
            const std::vector<Token> define_v = {
                Token::def_token(), Token::symbol_token("foo"),
                Token::colon_token(), Token::symbol_token("f64"),
                Token::singleassign_token(), Token::f64_token("3.5"),
                Token::semicolon_token()
            };

            /*  #q { { a : 1 } }
             *        ^ ^   ^
             *        | |   tk5  -- qdict_1d, :key cleared, :dict populated
             *        | tk3      -- qdict_1b, :key present, :dict empty
             *        tk2        -- qdict_1a, :key absent,  :dict empty
             */
            const std::vector<Token> qdict_v = {
                Token::quote_token(), Token::leftbrace_token(),
                Token::leftbrace_token(), Token::symbol_token("a"),
                Token::colon_token(), Token::i64_token("1"),
                Token::rightbrace_token(), Token::rightbrace_token()
            };

            /*  lambda ( ) { 1.5 }
             *  ^
             *  tk0  -- DExpectFormalArglistSsm at argl_0, :n_args 0
             */
            const std::vector<Token> arg0_v = {
                Token::lambda_token(), Token::leftparen_token(),
                Token::rightparen_token(), Token::leftbrace_token(),
                Token::f64_token("1.5"), Token::rightbrace_token()
            };

            /*  lambda ( x : f64 , y : f64 ) { 1.5 }
             *                  ^          ^
             *                  tk4        tk8
             *  -- :n_args 1 and 2: the arg[i] loop at one and two iterations.
             *     tk0 covers zero, above.
             */
            const std::vector<Token> arg2_v = {
                Token::lambda_token(), Token::leftparen_token(),
                Token::symbol_token("x"), Token::colon_token(),
                Token::symbol_token("f64"), Token::comma_token(),
                Token::symbol_token("y"), Token::colon_token(),
                Token::symbol_token("f64"), Token::rightparen_token(),
                Token::leftbrace_token(), Token::f64_token("1.5"),
                Token::rightbrace_token()
            };

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "reader2-parser-render"));

                /** step a fresh parser through @p tk_v, and render the top of
                 *  its stack after token @p i_tk.
                 **/
                auto check_step = [&rh, &log](const std::vector<Token> & tk_v,
                                              const char * name,
                                              std::size_t i_tk,
                                              std::uint32_t margin,
                                              const char * expect_deprecated,
                                              const char * expect_pretty)
                {
                    ParseFixture fx(std::string(name) + "." + std::to_string(i_tk)
                                    + "." + std::to_string(margin));

                    fx.parser_->begin_interactive_session();

                    for (std::size_t i = 0; i <= i_tk; ++i)
                        fx.parser_->on_token(tk_v[i]);

                    auto top = fx.parser_->top_ssm();

                    REQUIRE(bool(top));

                    auto pr = top.to_facet<APrintable>();

                    std::string deprecated
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_deprecated(pr, margin))));
                    std::string pretty
                        = scrub_tseq(scrub_type_id(
                              scrub_typevar(render_pretty(pr, margin))));

                    log && log(xtag("name", name), xtag("i_tk", i_tk),
                               xtag("margin", margin),
                               xtag("deprecated", deprecated),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == std::string(expect_pretty));
                    REHEARSE(rh, deprecated == std::string(expect_deprecated));
                };

                check_step(define_v, "define", 1, 200,
                           "<DDefineSsm :defstate def_2 :expect singleassign|colon :def_expr <DDefineExpr :lhs <DVariable :name \"foo\" :typeref <TypeRef :id \"\" :td null>>>>",
                           "<DDefineSsm :defstate def_2 :expect singleassign|colon :def_expr <DDefineExpr :lhs <DVariable :name \"foo\" :typeref <TypeRef :id \"\" :td null>>>>");

                check_step(define_v, "define", 1, 60,
                           "<DDefineSsm\n"
                           "  :defstate def_2\n"
                           "  :expect singleassign|colon\n"
                           "  :def_expr\n"
                           "    <DDefineExpr\n"
                           "      :lhs\n"
                           "        <DVariable\n"
                           "          :name \"foo\"\n"
                           "          :typeref <TypeRef :id \"\" :td null>>>>",
                           "<DDefineSsm\n"
                           "  :defstate def_2\n"
                           "  :expect singleassign|colon\n"
                           "  :def_expr\n"
                           "   <DDefineExpr\n"
                           "    :lhs\n"
                           "     <DVariable\n"
                           "      :name \"foo\"\n"
                           "      :typeref <TypeRef :id \"\" :td null>>>>");

                check_step(define_v, "define", 3, 200,
                           "<DDefineSsm\n"
                           "  :defstate def_4\n"
                           "  :expect singleassign\n"
                           "  :def_expr <DDefineExpr :lhs <DVariable :name \"foo\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>>",
                           "<DDefineSsm\n"
                           "  :defstate def_4\n"
                           "  :expect singleassign\n"
                           "  :def_expr <DDefineExpr :lhs <DVariable :name \"foo\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>>");

                check_step(define_v, "define", 3, 60,
                           "<DDefineSsm\n"
                           "  :defstate def_4\n"
                           "  :expect singleassign\n"
                           "  :def_expr\n"
                           "    <DDefineExpr\n"
                           "      :lhs\n"
                           "        <DVariable\n"
                           "          :name \"foo\"\n"
                           "          :typeref\n"
                           "            <TypeRef\n"
                           "              :id \"\"\n"
                           "              :td\n"
                           "                <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>>",
                           "<DDefineSsm\n"
                           "  :defstate def_4\n"
                           "  :expect singleassign\n"
                           "  :def_expr\n"
                           "   <DDefineExpr\n"
                           "    :lhs\n"
                           "     <DVariable\n"
                           "      :name \"foo\"\n"
                           "      :typeref\n"
                           "       <TypeRef\n"
                           "        :id \"\"\n"
                           "        :td\n"
                           "         <TypeDescr\n"
                           "          :id N\n"
                           "          :canonical_name double\n"
                           "          :complete 1\n"
                           "          :metatype atomic>>>>>");

                check_step(qdict_v, "qdict", 2, 200,
                           "<DExpectQDictSsm :state qdict_1a :expect symbol|rightbrace :dict { }>",
                           "<DExpectQDictSsm :state qdict_1a :expect symbol|rightbrace :dict {}>");

                check_step(qdict_v, "qdict", 2, 60,
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1a\n"
                           "  :expect symbol|rightbrace\n"
                           "  :dict { }>",
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1a\n"
                           "  :expect symbol|rightbrace\n"
                           "  :dict {}>");

                check_step(qdict_v, "qdict", 3, 200,
                           "<DExpectQDictSsm :state qdict_1b :expect colon :key a :dict { }>",
                           "<DExpectQDictSsm :state qdict_1b :expect colon :key a :dict {}>");

                check_step(qdict_v, "qdict", 3, 60,
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1b\n"
                           "  :expect colon\n"
                           "  :key a\n"
                           "  :dict { }>",
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1b\n"
                           "  :expect colon\n"
                           "  :key a\n"
                           "  :dict {}>");

                check_step(qdict_v, "qdict", 5, 200,
                           "<DExpectQDictSsm :state qdict_1d :expect semicolon|rightbrace :dict { a: 1; }>",
                           "<DExpectQDictSsm :state qdict_1d :expect semicolon|rightbrace :dict {a: 1;}>");

                check_step(qdict_v, "qdict", 5, 60,
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1d\n"
                           "  :expect semicolon|rightbrace\n"
                           "  :dict { a: 1; }>",
                           "<DExpectQDictSsm\n"
                           "  :state qdict_1d\n"
                           "  :expect semicolon|rightbrace\n"
                           "  :dict {a: 1;}>");

                /* DExpectFormalArglistSsm: a HAND-ROLLED two-pass printer,
                 * collapsed to struct_open() for its runtime 3 + n_args
                 * arity.  Its own framing is byte-identical on both
                 * protocols in every case here; the divergences at margins
                 * 60 and 30 all begin INSIDE <DVariable, i.e. in children
                 * converted earlier.
                 */
                check_step(arg0_v, "arg0", 0, 200,
                           "<DExpectFormalArglistSsm :fastate argl_0 :expect leftparen :n_args 0>",
                           "<DExpectFormalArglistSsm :fastate argl_0 :expect leftparen :n_args 0>");

                check_step(arg0_v, "arg0", 0, 30,
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_0\n"
                           "  :expect leftparen\n"
                           "  :n_args 0>",
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_0\n"
                           "  :expect leftparen\n"
                           "  :n_args 0>");

                check_step(arg2_v, "arg2", 4, 200,
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 1\n"
                           "  :arg[0] <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>",
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 1\n"
                           "  :arg[0] <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>");

                check_step(arg2_v, "arg2", 4, 60,
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 1\n"
                           "  :arg[0]\n"
                           "    <DVariable\n"
                           "      :name \"x\"\n"
                           "      :typeref\n"
                           "        <TypeRef\n"
                           "          :id \"\"\n"
                           "          :td\n"
                           "            <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>",
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 1\n"
                           "  :arg[0]\n"
                           "   <DVariable\n"
                           "    :name \"x\"\n"
                           "    :typeref\n"
                           "     <TypeRef\n"
                           "      :id \"\"\n"
                           "      :td\n"
                           "       <TypeDescr\n"
                           "        :id N\n"
                           "        :canonical_name double\n"
                           "        :complete 1\n"
                           "        :metatype atomic>>>>");

                check_step(arg2_v, "arg2", 8, 200,
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 2\n"
                           "  :arg[0] <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>\n"
                           "  :arg[1] <DVariable :name \"y\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>",
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 2\n"
                           "  :arg[0] <DVariable :name \"x\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>\n"
                           "  :arg[1] <DVariable :name \"y\" :typeref <TypeRef :id \"\" :td <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>");

                check_step(arg2_v, "arg2", 8, 60,
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 2\n"
                           "  :arg[0]\n"
                           "    <DVariable\n"
                           "      :name \"x\"\n"
                           "      :typeref\n"
                           "        <TypeRef\n"
                           "          :id \"\"\n"
                           "          :td\n"
                           "            <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>\n"
                           "  :arg[1]\n"
                           "    <DVariable\n"
                           "      :name \"y\"\n"
                           "      :typeref\n"
                           "        <TypeRef\n"
                           "          :id \"\"\n"
                           "          :td\n"
                           "            <TypeDescr :id N :canonical_name double :complete 1 :metatype atomic>>>>",
                           "<DExpectFormalArglistSsm\n"
                           "  :fastate argl_1b\n"
                           "  :expect comma|rightparen\n"
                           "  :n_args 2\n"
                           "  :arg[0]\n"
                           "   <DVariable\n"
                           "    :name \"x\"\n"
                           "    :typeref\n"
                           "     <TypeRef\n"
                           "      :id \"\"\n"
                           "      :td\n"
                           "       <TypeDescr\n"
                           "        :id N\n"
                           "        :canonical_name double\n"
                           "        :complete 1\n"
                           "        :metatype atomic>>>\n"
                           "  :arg[1]\n"
                           "   <DVariable\n"
                           "    :name \"y\"\n"
                           "    :typeref\n"
                           "     <TypeRef\n"
                           "      :id \"\"\n"
                           "      :td\n"
                           "       <TypeDescr\n"
                           "        :id N\n"
                           "        :canonical_name double\n"
                           "        :complete 1\n"
                           "        :metatype atomic>>>>");

            }
        }


    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
