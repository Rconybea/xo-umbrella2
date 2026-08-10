/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for the APrintable printers in xo-stringtable2.
 *
 * TEMPLATE for the remaining printers -- see
 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
 *
 * Each printer is rendered through BOTH protocols at the same margin:
 *
 *   deprecated  toppstr2(ppconfig, x)  -> ppdetail<obj<APrintable,D>>
 *                                      -> x.pretty_deprecated(ppii)
 *   new         toppstr(PpConfig, x)   -> Prettifier<obj<APrintable,D>>
 *                                      -> x.pretty(sink)
 *
 * and BOTH renderings are pinned.  Pinning both, rather than only asserting
 * they agree:
 *
 *   - the agreement check is SCAFFOLDING.  It cannot survive phase E, which
 *     deletes pretty_deprecated and with it render_deprecated().
 *   - the pinned `pretty` expectation is the coverage that REMAINS.
 *   - where the two deliberately differ (a reviewed rendering change, as for
 *     DList's "(...)" fallback), pinning both STATES the difference rather
 *     than failing on it.
 *
 * Margin is the case variable: it is what makes the protocols disagree if they
 * are going to.
 *
 * Uses UtestRehearser rather than INFO: renderings can run to many lines, and
 * INFO builds and stacks them on every iteration whether or not anything
 * fails.  REHEARSE records failures on a first pass and re-runs with logging
 * enabled only when needed.
 *
 * Expectations are OBSERVED, never predicted.
 */

#include "String.hpp"           /* pulls in IPrintable_DString.hpp */
#include "StringTable.hpp"      /* the public path to a DUniqueString */
#include "uniquestring/IPrintable_DUniqueString.hpp"
#include "init_stringtable2.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/indentlog/print/ppstr.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    using xo::scm::DString;
    using xo::scm::DUniqueString;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        namespace {
            struct Testcase_Render {
                Testcase_Render(std::uint32_t margin,
                                const char * text,
                                const char * expect_deprecated,
                                const char * expect_pretty)
                    : margin_{margin}, text_{text},
                      expect_deprecated_{expect_deprecated},
                      expect_pretty_{expect_pretty} {}

                /** right margin, applied to BOTH protocols **/
                std::uint32_t margin_;
                /** string content to render **/
                std::string text_;
                /** OBSERVED rendering via pretty_deprecated.
                 *  Delete at phase E along with render_deprecated().
                 **/
                std::string expect_deprecated_;
                /** OBSERVED rendering via pretty.  The assertion that outlives
                 *  phase E.
                 **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Render>
            s_testcase_v = {
                /*              margin  text                 deprecated           pretty */
                Testcase_Render(200,    "hello",             "hello",             "hello"),
                Testcase_Render(20,     "hello",             "hello",             "hello"),
                Testcase_Render(4,      "hello",             "hello",             "hello"),
                Testcase_Render(200,    "",                  "",                  ""),
                Testcase_Render(200,    "with spaces in it", "with spaces in it", "with spaces in it"),
                Testcase_Render(8,      "with spaces in it", "with spaces in it", "with spaces in it"),
            };

            /** render @p x through the DEPRECATED two-pass protocol.
             *
             *  DELETE AT PHASE E, with expect_deprecated_ and its REHEARSE.
             **/
            template <typename T>
            std::string
            render_deprecated(const T & x, std::uint32_t margin) {
                xo::print::ppconfig ppc = xo::print::ppconfig::ugly();
                ppc.right_margin_ = margin;

                return xo::toppstr2(ppc, x);
            }

            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string
            render_pretty(const T & x, std::uint32_t margin) {
                return toppstr(PpConfig::scratch_plain(margin), x);
            }
        } /*namespace*/

        TEST_CASE("DString-render", "[printable][DString]")
        {
            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DString-render"));

                for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                    const Testcase_Render & tc = s_testcase_v[i_tc];

                    ArenaConfig cfg { .name_ = "testarena." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    DString * s = DString::from_cstr(alloc, tc.text_.c_str());
                    auto p = with_facet<APrintable>::mkobj(s);

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc),
                               xtag("margin", tc.margin_),
                               xtag("text", tc.text_),
                               xtag("deprecated", deprecated),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }

        TEST_CASE("DUniqueString-render", "[printable][DUniqueString]")
        {
            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DUniqueString-render"));

                for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                    const Testcase_Render & tc = s_testcase_v[i_tc];

                    xo::scm::StringTable table(1024);
                    const DUniqueString * u = table.intern(tc.text_);
                    auto p = with_facet<APrintable>::mkobj(const_cast<DUniqueString *>(u));

                    std::string deprecated = render_deprecated(p, tc.margin_);
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc),
                               xtag("margin", tc.margin_),
                               xtag("text", tc.text_),
                               xtag("deprecated", deprecated),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                    REHEARSE(rh, deprecated == tc.expect_deprecated_);
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
