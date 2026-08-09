/* @file printable_render.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Phase C verification for the APrintable printers in xo-stringtable2.
 *
 * Each converted printer must render IDENTICALLY through both protocols at the
 * same margin:
 *
 *   deprecated  toppstr2(ppconfig, x)  -> ppdetail<obj<APrintable,D>>
 *                                      -> x.pretty_deprecated(ppii)
 *   new         toppstr(PpConfig, x)   -> Prettifier<obj<APrintable,D>>
 *                                      -> x.pretty(sink)
 *
 * The margin is the case variable: it is what makes the two protocols disagree
 * if they are going to.  DString/DUniqueString are a degenerate case -- neither
 * ever breaks -- which is why they are first: the cycle is being established
 * here, not stress-tested.
 *
 * Expectations are OBSERVED, never predicted.
 * See .xo-backlog/xo-printable2/issues/01-aprintable-pretty-ppsink.md
 */

#include "String.hpp"           /* pulls in IPrintable_DString.hpp */
#include "StringTable.hpp"      /* the public path to a DUniqueString */
#include "uniquestring/IPrintable_DUniqueString.hpp"
#include "init_stringtable2.hpp"
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/indentlog/print/ppstr.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
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
        namespace {
            struct Testcase_Render {
                Testcase_Render(std::uint32_t margin, const char * text,
                                const char * expected)
                    : margin_{margin}, text_{text}, expected_{expected} {}

                /** right margin to render at, in BOTH protocols **/
                std::uint32_t margin_;
                /** string content to render **/
                std::string text_;
                /** OBSERVED rendering; pinned so the test survives phase E,
                 *  when pretty_deprecated (and render_deprecated) go away
                 **/
                std::string expected_;
            };

            std::vector<Testcase_Render>
            s_testcase_v = {
                Testcase_Render(200, "hello", "hello"),
                Testcase_Render(20,  "hello", "hello"),
                Testcase_Render(4,   "hello", "hello"),          /* narrower than content */
                Testcase_Render(200, "", ""),
                Testcase_Render(200, "with spaces in it", "with spaces in it"),
                Testcase_Render(8,   "with spaces in it", "with spaces in it"),
            };

            /** render @p x through the DEPRECATED two-pass protocol **/
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
                return xo::pp::toppstr(xo::pp::PpConfig().with_soft_right_margin(margin),
                                       x);
            }
        } /*namespace*/

        TEST_CASE("DString-render-both-protocols", "[printable][DString]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_Render & tc = s_testcase_v[i_tc];

                ArenaConfig cfg { .name_ = "testarena." + std::to_string(i_tc),
                                  .size_ = 4*1024 };
                DArena arena = DArena::map(cfg);
                auto alloc = with_facet<AAllocator>::mkobj(&arena);

                DString * s = DString::from_cstr(alloc, tc.text_.c_str());
                auto p = with_facet<APrintable>::mkobj(s);

                std::string legacy = render_deprecated(p, tc.margin_);
                std::string modern = render_pretty(p, tc.margin_);

                INFO("i_tc=" << i_tc << " margin=" << tc.margin_
                     << " text=[" << tc.text_ << "]"
                     << " deprecated=[" << legacy << "]"
                     << " pretty=[" << modern << "]");

                REQUIRE(modern == legacy);
                REQUIRE(modern == tc.expected_);
            }
        }

        TEST_CASE("DUniqueString-render-both-protocols", "[printable][DUniqueString]")
        {
            for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                const Testcase_Render & tc = s_testcase_v[i_tc];

                xo::scm::StringTable table(1024);
                const DUniqueString * u = table.intern(tc.text_);
                auto p = with_facet<APrintable>::mkobj(const_cast<DUniqueString *>(u));

                std::string legacy = render_deprecated(p, tc.margin_);
                std::string modern = render_pretty(p, tc.margin_);

                INFO("i_tc=" << i_tc << " margin=" << tc.margin_
                     << " text=[" << tc.text_ << "]"
                     << " deprecated=[" << legacy << "]"
                     << " pretty=[" << modern << "]");

                REQUIRE(modern == legacy);
                REQUIRE(modern == tc.expected_);
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end printable_render.test.cpp */
