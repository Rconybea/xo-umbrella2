/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for xo-object2's LEAF printers.
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

#include <xo/object2/DBoolean.hpp>
#include <xo/object2/DFloat.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/object2/RuntimeError.hpp>
#include <xo/object2/number/IPrintable_DInteger.hpp>
#include <xo/object2/number/IPrintable_DFloat.hpp>
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
#include <vector>

namespace xo {
    using xo::scm::DBoolean;
    using xo::scm::DFloat;
    using xo::scm::DInteger;
    using xo::scm::DRuntimeError;
    using xo::scm::DString;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
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
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
