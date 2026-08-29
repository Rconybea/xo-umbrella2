/** @file DObjectEvent.test.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  DObjectEvent is DTypedEvent<obj<AGCObject>>: a timestamp plus a
 *  garbage-collected payload.
 *
 *  The payload here is a DString (xo-stringtable2), chosen because
 *  xo-reactor2 already depends on it and it supplies both of the facet
 *  impls DObjectEvent needs of its value: AGCObject (to hold it) and
 *  APrintable (to render it).
 *
 *  Renderings are PINNED, and were OBSERVED rather than predicted -- margin
 *  is the case variable, since that is what makes a printer break lines.
 *
 *  Uses UtestRehearser rather than INFO: a rendering can run to several
 *  lines, and INFO builds and stacks them every iteration whether or not
 *  anything fails.  REHEARSE records a failure on a quiet first pass and
 *  re-runs with logging enabled only when needed.
 */

#include "event/DObjectEvent.hpp"
#include "event/IPrintable_DObjectEvent.hpp"
#include <xo/reactor2/init_reactor2.hpp>
#include <xo/stringtable2/DString.hpp>
#include <xo/stringtable2/string/IPrintable_DString.hpp>
#include <xo/stringtable2/string/IGCObject_DString.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/testutil/UtestRehearser.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace xo {
    using xo::process::DObjectEvent;
    using xo::scm::DString;
    using xo::print::APrintable;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::time::utc_nanos;

    namespace ut {
        using xo::pp::PpConfig;
        using xo::pp::toppstr;
        using xo::pp::scope;
        using xo::pp::xtag;

        /* registers reactor2's setup code, to be run when
         * Subsystem::initialize_all() does.  reactor2's require() chains to
         * stringtable2 (and reflect), so this also registers the facet impls
         * for the DString payload that DObjectEvent::pretty() pivots to.
         */
        static InitEvidence s_init = InitSubsys<S_reactor2_tag>::require();

        namespace {
            /** a fixed, arbitrary timestamp -- the tests pin its rendering,
             *  so it must not be 'now'
             **/
            utc_nanos test_tm() {
                return xo::time::timeutil::ymd_midnight(20260828);
            }

            /** render @p x through pretty(PpSink&) **/
            template <typename T>
            std::string render_pretty(const T & x, std::uint32_t margin) {
                return toppstr(PpConfig::scratch_plain(margin), x);
            }

            struct Testcase_Render {
                Testcase_Render(std::uint32_t margin,
                                const char * text,
                                const char * expect_pretty)
                    : margin_{margin}, text_{text},
                      expect_pretty_{expect_pretty} {}

                /** right margin **/
                std::uint32_t margin_;
                /** payload string content **/
                std::string text_;
                /** OBSERVED rendering **/
                std::string expect_pretty_;
            };

            std::vector<Testcase_Render>
            s_testcase_v = {
                /* wide margin: the whole struct fits on one line */
                Testcase_Render(200, "hello",
                                "<DObjectEvent"
                                " :tm 2026-08-28 00:00:00.000000000"
                                " :value hello>"),
                /* narrower: one field per line */
                Testcase_Render(40, "hello",
                                "<DObjectEvent\n"
                                "  :tm 2026-08-28 00:00:00.000000000\n"
                                "  :value hello>"),
                /* narrower still: the timestamp no longer fits beside its
                 * tag, so it breaks onto its own continuation line
                 */
                Testcase_Render(16, "hello",
                                "<DObjectEvent\n"
                                "  :tm\n"
                                "   2026-08-28 00:00:00.000000000\n"
                                "  :value hello>"),
                /* empty payload still renders the field, with empty content */
                Testcase_Render(200, "",
                                "<DObjectEvent"
                                " :tm 2026-08-28 00:00:00.000000000"
                                " :value >"),
            };
        } /*namespace*/

        TEST_CASE("DObjectEvent-ctor", "[reactor2][DObjectEvent]")
        {
            ArenaConfig cfg { .name_ = "testarena", .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DString * s = DString::from_cstr(alloc, "hello");
            REQUIRE(s != nullptr);

            auto value = with_facet<AGCObject>::mkobj(s);
            utc_nanos tm = test_tm();

            DObjectEvent ev(tm, value);

            /* timestamp and payload both survive construction */
            CHECK(ev.tm() == tm);
            CHECK(ev.value().data() == static_cast<void *>(s));
        }

        TEST_CASE("DObjectEvent-default-ctor", "[reactor2][DObjectEvent]")
        {
            DObjectEvent ev;

            /* default event carries no payload */
            CHECK(!ev.value());
            CHECK(ev.value().data() == nullptr);
        }

        TEST_CASE("DObjectEvent-render", "[reactor2][DObjectEvent][printable]")
        {
            UtestRehearser rh;

            for (auto _ : rh) {
                scope log(XO_DEBUG2_(rh.enable_debug(), "DObjectEvent-render"));

                for (std::size_t i_tc = 0, n_tc = s_testcase_v.size(); i_tc < n_tc; ++i_tc) {
                    const Testcase_Render & tc = s_testcase_v[i_tc];

                    ArenaConfig cfg { .name_ = "testarena." + std::to_string(i_tc),
                                      .size_ = 4*1024 };
                    DArena arena = DArena::map(cfg);
                    auto alloc = with_facet<AAllocator>::mkobj(&arena);

                    DString * s = DString::from_cstr(alloc, tc.text_.c_str());
                    DObjectEvent ev(test_tm(), with_facet<AGCObject>::mkobj(s));

                    auto p = with_facet<APrintable>::mkobj(&ev);
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

/* end DObjectEvent.test.cpp */
