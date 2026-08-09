/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-object2's printers: the leaves (DInteger,
 * DBoolean, DFloat), then DRuntimeError (a struct), DArray (a sequence) and
 * DDictionary (a keyed sequence).
 *
 * Follows the template in xo-stringtable2/utest/printable_render.test.cpp --
 * see .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md for why
 * both renderings are pinned rather than only asserted equal.
 *
 * DList is verified separately, in xo-gc/utest/Object2.test.cpp, because it
 * needs a collector fixture and its elements exercise these leaves.
 *
 * The interesting one here is DFloat: legacy formatted a double through
 * ppstate::write(double), the sink through Prettifier<double>.  Those are
 * different code paths and "prints a double" is not evidence they agree --
 * hence the range of values below rather than a single 1.5.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include <xo/object2/DArray.hpp>
#include <xo/object2/DBoolean.hpp>
#include <xo/object2/DDictionary.hpp>
#include <xo/object2/DFloat.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/object2/RuntimeError.hpp>
#include <xo/object2/array/IGCObject_DArray.hpp>
#include <xo/object2/array/IPrintable_DArray.hpp>
#include <xo/object2/dictionary/IGCObject_DDictionary.hpp>
#include <xo/object2/dictionary/IPrintable_DDictionary.hpp>
#include <xo/object2/number/IPrintable_DInteger.hpp>
#include <xo/object2/number/IPrintable_DFloat.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/object2/boolean/IPrintable_DBoolean.hpp>
#include <xo/object2/SetupObject2.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/indentlog/print/ppstr.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace xo {
    using xo::scm::DArray;
    using xo::scm::DBoolean;
    using xo::scm::DDictionary;
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::scm::DRuntimeError;
    using xo::scm::DString;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;

    namespace ut {
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            /** render @p x through the DEPRECATED two-pass protocol.
             *  DELETE AT PHASE E, with expect_deprecated_ and its REHEARSE.
             *
             *  NOT ppconfig::ugly(), which the leaf tests inherited from the
             *  xo-stringtable2 template: ugly() sets indent_width_ = 0, so a
             *  broken struct's fields land in column 0 and the comparison
             *  silently stops being about indent at all.  Default ppconfig has
             *  indent_width_ = 2, matching xo::pp::PpConfig.
             *
             *  Color is a process-wide global in both stacks, not part of
             *  either config object, and both now default ON.  Suppressed here
             *  so the expectations pin LAYOUT rather than ANSI escapes.
             **/
            template <typename T>
            std::string
            render_deprecated(const T & x, std::uint32_t margin) {
                xo::print::ppconfig ppc;
                ppc.right_margin_ = margin;

                bool orig_color = xo::tag_config::tag_color_enabled;
                xo::tag_config::tag_color_enabled = false;

                std::string retval = xo::toppstr2(ppc, x);

                xo::tag_config::tag_color_enabled = orig_color;

                return retval;
            }

            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string
            render_pretty(const T & x, std::uint32_t margin) {
                /* PpStyle::plain(): struct field names are yellow by default
                 * (PpStyle.hpp), and this test is about LAYOUT.  Per-render via
                 * the config rather than by flipping a global, so it cannot
                 * leak into whatever runs next.
                 */
                return xo::pp::toppstr(xo::pp::PpConfig()
                                       .with_soft_right_margin(margin)
                                       .with_style(xo::pp::PpStyle::plain()),
                                       x);
            }

            template <typename V>
            struct Testcase_Leaf {
                Testcase_Leaf(V value, const char * expect_deprecated, const char * expect_pretty)
                    : value_{value},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                V value_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Leaf<long>>
            s_integer_v = {
                Testcase_Leaf<long>(0,          "0",          "0"),
                Testcase_Leaf<long>(1,          "1",          "1"),
                Testcase_Leaf<long>(-1,         "-1",         "-1"),
                Testcase_Leaf<long>(1234567890, "1234567890", "1234567890"),
            };

            std::vector<Testcase_Leaf<bool>>
            s_boolean_v = {
                Testcase_Leaf<bool>(true,  "true",  "true"),
                Testcase_Leaf<bool>(false, "false", "false"),
            };

            /* double formatting is where the two paths could diverge */
            std::vector<Testcase_Leaf<double>>
            s_float_v = {
                Testcase_Leaf<double>(0.0, "0", "0"),
                Testcase_Leaf<double>(1.5, "1.5", "1.5"),
                Testcase_Leaf<double>(-2.25, "-2.25", "-2.25"),
                Testcase_Leaf<double>(1.0/3.0, "0.333333", "0.333333"),
                Testcase_Leaf<double>(1e20, "1e+20", "1e+20"),
                Testcase_Leaf<double>(1e-20, "1e-20", "1e-20"),
            };
            /** DRuntimeError is the first STRUCTURED printer verified here:
             *  two DString fields inside a pretty_struct.  Unlike the leaves
             *  above, its rendering DEPENDS ON THE MARGIN, so the margin is
             *  part of the case.
             **/
            struct Testcase_Error {
                Testcase_Error(std::uint32_t margin,
                               const char * src, const char * err,
                               const char * expect_deprecated,
                               const char * expect_pretty)
                    : margin_{margin}, src_{src}, err_{err},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                std::uint32_t margin_;
                const char * src_;
                const char * err_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Error>
            s_error_v = {
                /* fits: one line.  identical. */
                Testcase_Error(80, "DRuntimeError::make", "bad argument",
                               "<DRuntimeError :src DRuntimeError::make :err bad argument>",
                               "<DRuntimeError :src DRuntimeError::make :err bad argument>"),
                /* struct breaks, each field still fits its line.  identical --
                 * the struct-level indent agrees at 2.
                 */
                Testcase_Error(40, "DRuntimeError::make", "bad argument",
                               "<DRuntimeError\n"
                               "  :src DRuntimeError::make\n"
                               "  :err bad argument>",
                               "<DRuntimeError\n"
                               "  :src DRuntimeError::make\n"
                               "  :err bad argument>"),
                /* narrow enough that each field breaks from its own value.
                 *
                 * REVIEWED DIVERGENCE, deliberate: the continuation indent for
                 * a broken value is 4 under legacy and 3 under ppsink.  Legacy
                 * adds a further indent_width_ (2); ppsink adds
                 * xo::pp::tag_config::value_offset, which is 1 (tag.hpp:46), so
                 * the value hangs one column past its ":name" rather than lining
                 * up with the next nesting level.  Deliberate in ppsink and
                 * configurable there, where legacy's was neither.
                 */
                Testcase_Error(16, "DRuntimeError::make", "bad argument",
                               "<DRuntimeError\n"
                               "  :src\n"
                               "    DRuntimeError::make\n"
                               "  :err\n"
                               "    bad argument>",
                               "<DRuntimeError\n"
                               "  :src\n"
                               "   DRuntimeError::make\n"
                               "  :err\n"
                               "   bad argument>"),
            };
            /** DArray is the first SEQUENCE printer verified here.  Unlike
             *  DRuntimeError its arity is not fixed, so the margin decides both
             *  whether it breaks and how many elements share a line.
             *
             *  Note this does NOT render like Prettifier<vector<T>>, which
             *  indents a broken sequence by a full nesting level
             *  ("[1,\n  2,\n  3]", pinned in
             *  xo-indentlog2/utest/PrettyVector.test.cpp).  DArray aligns to
             *  element 0 instead -- RC's call, matching what legacy rendered.
             *
             *  Elements are DIntegers -- leaves already pinned above, so any
             *  difference here is the array's own framing, not an element's.
             **/
            struct Testcase_Array {
                Testcase_Array(std::uint32_t margin,
                               std::vector<long> elt_v,
                               const char * expect_deprecated,
                               const char * expect_pretty)
                    : margin_{margin}, elt_v_{std::move(elt_v)},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                std::uint32_t margin_;
                std::vector<long> elt_v_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Array>
            s_array_v = {
                /* degenerate: empty group must not break, and must not emit a
                 * separator it has no elements to separate.
                 */
                Testcase_Array(80, {}, "[]", "[]"),
                Testcase_Array(80, {1}, "[1]", "[1]"),
                /* fits: one line, space-separated.  identical. */
                Testcase_Array(80, {1, 2, 3}, "[1 2 3]", "[1 2 3]"),
                /* REVIEWED DIVERGENCE, deliberate: both stacks align elements
                 * 2..n under element 0, but they get there differently.
                 *
                 * Legacy pads -- it writes a space after "[" so that element 0
                 * starts at the continuation indent ("[ 100" / "  200").
                 * ppsink offsets instead: begin(1) credits the bracket, so
                 * element 0 follows "[" immediately and the continuation indent
                 * is 1 to match it.  Same alignment, one column left, and no
                 * whitespace inside a rendering that claims to have none.
                 */
                Testcase_Array(8, {100, 200, 300},
                               "[ 100\n  200\n  300]",
                               "[100\n 200\n 300]"),
                /* margin narrower than a single element: no further recourse,
                 * so the rendering is unchanged from margin 8 rather than
                 * degenerating.
                 */
                Testcase_Array(4, {100, 200, 300},
                               "[ 100\n  200\n  300]",
                               "[100\n 200\n 300]"),
            };
            /** DDictionary is a KEYED sequence: like DArray its arity is
             *  variable, but each element is itself two printers plus fixed
             *  punctuation, so the same margin breaks it sooner.
             *
             *  Keys are DStrings, which render UNQUOTED (DString::pretty ->
             *  sink.pp(const char *)); values here are DIntegers.  Both are
             *  pinned already, so any difference below is the dictionary's own
             *  framing.
             *
             *  Each entry is its own group with a break point after the ":", so
             *  a value that will not fit folds onto the next line rather than
             *  starting inline wherever the key ended.  RC's call: the key
             *  could just as well have been forty characters long, and the
             *  layout must not depend on that.
             **/
            struct Testcase_Dict {
                using entry_type = std::pair<const char *, long>;

                Testcase_Dict(std::uint32_t margin,
                              std::vector<entry_type> entry_v,
                              const char * expect_deprecated,
                              const char * expect_pretty)
                    : margin_{margin}, entry_v_{std::move(entry_v)},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                std::uint32_t margin_;
                std::vector<entry_type> entry_v_;
                /** OBSERVED via pretty_deprecated; delete at phase E **/
                std::string expect_deprecated_;
                /** OBSERVED via pretty; outlives phase E **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Dict>
            s_dict_v = {
                /* degenerate: empty group must not break, and must not emit a
                 * separator it has no entries to separate.  Legacy still pads,
                 * so an empty dictionary is TWO characters there and one here.
                 */
                Testcase_Dict(80, {}, "{ }", "{}"),
                Testcase_Dict(80, {{"a", 1}}, "{ a: 1; }", "{a: 1;}"),
                /* fits: one line.  ";" TERMINATES each entry rather than
                 * separating them, so the last one carries it too -- legacy's
                 * shape, kept.
                 */
                Testcase_Dict(80, {{"a", 1}, {"bb", 22}},
                              "{ a: 1; bb: 22; }", "{a: 1; bb: 22;}"),
                /* REVIEWED DIVERGENCE, deliberate, and the same one DArray
                 * settled: both stacks align entries 2..n under entry 0, but
                 * legacy gets there by PADDING (a space after "{", so entry 0
                 * starts at the continuation indent) where ppsink OFFSETS
                 * (begin(1) credits the brace).  Same alignment, one column
                 * left, and no interior whitespace.
                 */
                Testcase_Dict(12, {{"a", 1}, {"bb", 22}},
                              "{ a: 1;\n  bb: 22; }",
                              "{a: 1;\n bb: 22;}"),
                /* margin narrower than a single entry.  Unlike DArray -- whose
                 * elements are atomic, so a too-narrow margin has no further
                 * recourse -- an entry CAN still fold after its ":", and does.
                 * Legacy has no such break point, so it is unchanged from
                 * margin 12 while ppsink degrades one step further.
                 */
                Testcase_Dict(4, {{"a", 1}, {"bb", 22}},
                              "{ a: 1;\n  bb: 22; }",
                              "{a:\n  1;\n bb:\n  22;}"),
                /* A key wide enough to crowd the margin on its own.  Each entry
                 * is its own group with a break point after the ":", so the
                 * value FOLDS onto the next line at entry-indent + 1 and the
                 * layout stops depending on how long the key was.
                 *
                 * REVIEWED DIVERGENCE, deliberate: legacy has no such break
                 * point, so it emits a 41-column line at a margin of 30 and
                 * renders margin 12 identically -- it simply cannot fold here.
                 */
                Testcase_Dict(30, {{"keep_going_until_close_to_margin", 12345}, {"n", 3}},
                              "{ keep_going_until_close_to_margin: 12345;\n"
                              "  n: 3; }",
                              "{keep_going_until_close_to_margin:\n"
                              "  12345;\n"
                              " n: 3;}"),
            };
        } /*namespace*/

        TEST_CASE("DInteger-render", "[printable][DInteger]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DInteger-render"));

                for (std::size_t i_tc = 0, n_tc = s_integer_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_integer_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.int." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    auto p = with_facet<APrintable>::mkobj(DInteger::_box(alloc, tc.value_));

                    std::string deprecated = render_deprecated(p, 80);
                    std::string pretty = render_pretty(p, 80);

                    log && log(xtag("i_tc", i_tc), xtag("value", tc.value_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DBoolean-render", "[printable][DBoolean]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DBoolean-render"));

                for (std::size_t i_tc = 0, n_tc = s_boolean_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_boolean_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.bool." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    auto p = with_facet<APrintable>::mkobj(DBoolean::_box(alloc, tc.value_));

                    std::string deprecated = render_deprecated(p, 80);
                    std::string pretty = render_pretty(p, 80);

                    log && log(xtag("i_tc", i_tc), xtag("value", tc.value_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DFloat-render", "[printable][DFloat]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DFloat-render"));

                for (std::size_t i_tc = 0, n_tc = s_float_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_float_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.float." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    auto p = with_facet<APrintable>::mkobj(DFloat::_box(alloc, tc.value_));

                    std::string deprecated = render_deprecated(p, 80);
                    std::string pretty = render_pretty(p, 80);

                    log && log(xtag("i_tc", i_tc), xtag("value", tc.value_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }
        TEST_CASE("DRuntimeError-render", "[printable][DRuntimeError]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DRuntimeError-render"));

                for (std::size_t i_tc = 0, n_tc = s_error_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_error_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.error." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    DRuntimeError * e
                        = DRuntimeError::_make(alloc,
                                               DString::from_cstr(alloc, tc.src_),
                                               DString::from_cstr(alloc, tc.err_));

                    auto p = with_facet<APrintable>::mkobj(e);

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DArray-render", "[printable][DArray]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DArray-render"));

                for (std::size_t i_tc = 0, n_tc = s_array_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_array_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.array." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    DArray * arr = DArray::_empty(alloc, tc.elt_v_.size());

                    REQUIRE(arr != nullptr);

                    for (long elt : tc.elt_v_)
                        REQUIRE(arr->push_back(alloc, DInteger::box<AGCObject>(alloc, elt)));

                    auto p = with_facet<APrintable>::mkobj(arr);

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DDictionary-render", "[printable][DDictionary]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DDictionary-render"));

                for (std::size_t i_tc = 0, n_tc = s_dict_v.size(); i_tc < n_tc; ++i_tc) {
                    const auto & tc = s_dict_v[i_tc];

                    ArenaConfig cfg { .name_ = "utest.dict." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    DDictionary * dict = DDictionary::empty(alloc, tc.entry_v_.size());

                    REQUIRE(dict != nullptr);

                    for (const auto & entry : tc.entry_v_) {
                        REQUIRE(dict->upsert_cstr(alloc, entry.first,
                                                  DInteger::box<AGCObject>(alloc, entry.second)));
                    }

                    auto p = with_facet<APrintable>::mkobj(dict);

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc), xtag("margin", tc.margin_),
                               xtag("deprecated", deprecated), xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        /** As with DArray, a flat dictionary shows only that the group breaks.
         *  Nesting one dictionary inside another is what shows the inner
         *  group's offset COMPOSING with the enclosing indent rather than
         *  resetting to column 0 -- and, at margin 16, the entry folding after
         *  its key before the value is asked to break at all.
         **/
        TEST_CASE("DDictionary-render-nested", "[printable][DDictionary]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            ArenaConfig cfg { .name_ = "utest.dict.nested", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            /* {k: {a: 1; b: 2;}; n: 3;} */
            DDictionary * inner = DDictionary::empty(alloc, 2);
            REQUIRE(inner->upsert_cstr(alloc, "a", DInteger::box<AGCObject>(alloc, 1)));
            REQUIRE(inner->upsert_cstr(alloc, "b", DInteger::box<AGCObject>(alloc, 2)));

            DDictionary * outer = DDictionary::empty(alloc, 2);
            REQUIRE(outer->upsert_cstr(alloc, "k",
                                       obj<AGCObject,DDictionary>(inner)));
            REQUIRE(outer->upsert_cstr(alloc, "n", DInteger::box<AGCObject>(alloc, 3)));

            auto p = with_facet<APrintable>::mkobj(outer);

            /* fits */
            REQUIRE(render_pretty(p, 80) == "{k: {a: 1; b: 2;}; n: 3;}");
            REQUIRE(render_deprecated(p, 80) == "{ k: { a: 1; b: 2; }; n: 3; }");

            /* outer breaks, inner still fits: entry 1 lines up under entry 0,
             * i.e. indent 1.
             */
            REQUIRE(render_pretty(p, 20) == ("{k: {a: 1; b: 2;};\n"
                                             " n: 3;}"));
            REQUIRE(render_deprecated(p, 20) == ("{ k: { a: 1; b: 2; };\n"
                                                 "  n: 3; }"));

            /* the entry no longer fits, so it breaks after "k:" and the value
             * starts a fresh line at entry-indent + 1 = 2.  The nested value
             * is thereby placed independently of the key's width -- which is
             * the whole point of giving each entry its own group.
             *
             * Legacy has no break point after the key, so it keeps the value
             * inline and lets the INNER dictionary break instead, indenting to
             * that dictionary's own column (7).
             */
            REQUIRE(render_pretty(p, 16) == ("{k:\n"
                                             "  {a: 1; b: 2;};\n"
                                             " n: 3;}"));
            REQUIRE(render_deprecated(p, 16) == ("{ k: { a: 1;\n"
                                                 "       b: 2; };\n"
                                                 "  n: 3; }"));

            /* Both levels break.  The inner dictionary opens at the running
             * indent (2), so its own begin(1) puts its entries at 3 -- the
             * offset COMPOSES with the enclosing indent rather than resetting.
             * That is the assertion a flat dictionary cannot make.
             *
             * Legacy is UNCHANGED from margin 16: with no fold after the key it
             * has no further recourse.
             */
            REQUIRE(render_pretty(p, 10) == ("{k:\n"
                                             "  {a: 1;\n"
                                             "   b: 2;};\n"
                                             " n: 3;}"));
            REQUIRE(render_deprecated(p, 10) == ("{ k: { a: 1;\n"
                                                 "       b: 2; };\n"
                                                 "  n: 3; }"));
        }

        /** A flat array shows that the group breaks; only a NESTED one shows
         *  that an inner group indents relative to its parent rather than to
         *  column 0.  That is the property a sequence printer can get wrong
         *  without any of the cases above noticing.
         **/
        TEST_CASE("DArray-render-nested", "[printable][DArray]")
        {
            REQUIRE(xo::scm::SetupObject2::register_facets());

            ArenaConfig cfg { .name_ = "utest.array.nested", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            /* [[100 200] [300]] */
            DArray * inner1 = DArray::_empty(alloc, 2);
            REQUIRE(inner1->push_back(alloc, DInteger::box<AGCObject>(alloc, 100)));
            REQUIRE(inner1->push_back(alloc, DInteger::box<AGCObject>(alloc, 200)));

            DArray * inner2 = DArray::_empty(alloc, 1);
            REQUIRE(inner2->push_back(alloc, DInteger::box<AGCObject>(alloc, 300)));

            DArray * outer = DArray::_empty(alloc, 2);
            REQUIRE(outer->push_back(alloc, with_facet<AGCObject>::mkobj(inner1)));
            REQUIRE(outer->push_back(alloc, with_facet<AGCObject>::mkobj(inner2)));

            auto p = with_facet<APrintable>::mkobj(outer);

            /* fits */
            REQUIRE(render_pretty(p, 80) == "[[100 200] [300]]");
            REQUIRE(render_deprecated(p, 80) == "[[100 200] [300]]");

            /* outer breaks, inner arrays still fit: element 1 lines up under
             * element 0, i.e. indent 1 -- not column 0, and not the enclosing
             * indent + a nesting level.
             */
            REQUIRE(render_pretty(p, 12) == ("[[100 200]\n"
                                             " [300]]"));
            REQUIRE(render_deprecated(p, 12) == ("[ [100 200]\n"
                                                 "  [300]]"));

            /* both levels break.  The inner array's own "[" sits at column 1, so
             * its elements align at column 2: the offset composes with the
             * enclosing indent rather than resetting.  This is the assertion a
             * flat array cannot make.
             */
            REQUIRE(render_pretty(p, 6) == ("[[100\n"
                                            "  200]\n"
                                            " [300]]"));
            REQUIRE(render_deprecated(p, 6) == ("[ [ 100\n"
                                                "    200]\n"
                                                "  [ 300]]"));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
