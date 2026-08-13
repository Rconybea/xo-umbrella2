/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for the APrintable printers in xo-stringtable2.
 *
 * TEMPLATE for the remaining printers -- see
 * .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
 *
 * Each printer is rendered through
 *
 *   toppstr(PpConfig, x)  -> Prettifier<obj<APrintable,D>>  -> x.pretty(sink)
 *
 * and the rendering is PINNED, not merely smoke-tested.
 *
 * Until phase E these tables carried a second expectation per case, observed
 * through the deprecated two-pass protocol, so that a conversion could be
 * checked against the rendering it replaced.  That scaffolding was deleted
 * with pretty_deprecated itself; the pinned `pretty` expectations are what
 * remain, and they are the coverage.  Where the two protocols deliberately
 * differed (a reviewed rendering change, as for DList's "(...)" fallback),
 * the surviving expectation is the NEW one.
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
                                const char * expect_pretty)
                    : margin_{margin}, text_{text},
                      expect_pretty_{expect_pretty} {}

                /** right margin, applied to BOTH protocols **/
                std::uint32_t margin_;
                /** string content to render **/
                std::string text_;
                /** OBSERVED rendering via pretty.  The assertion that outlives
                 *  phase E.
                 **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Render>
            s_testcase_v = {
                /*              margin  text                 pretty */
                Testcase_Render(200,    "hello",             "hello"),
                Testcase_Render(20,     "hello",             "hello"),
                Testcase_Render(4,      "hello",             "hello"),
                Testcase_Render(200,    "",                  ""),
                Testcase_Render(200,    "with spaces in it", "with spaces in it"),
                Testcase_Render(8,      "with spaces in it", "with spaces in it"),
            };

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
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc),
                               xtag("margin", tc.margin_),
                               xtag("text", tc.text_),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
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
                    std::string pretty = render_pretty(p, tc.margin_);

                    log && log(xtag("i_tc", i_tc),
                               xtag("margin", tc.margin_),
                               xtag("text", tc.text_),
                               xtag("pretty", pretty));

                    REHEARSE(rh, pretty == tc.expect_pretty_);
                }
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
