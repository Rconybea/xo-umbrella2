/* @file object_pretty.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Object::pretty(PpSink&) -- the structured rendering path
 * (.xo-backlog/xo-alloc/issues/01-object-pretty-ppsink.md).
 *
 * display_baseline.test.cpp pins that the conversion changes no FLAT output.
 * This file pins the thing the conversion is actually for, which flat output
 * cannot show: a gp<Object> nested inside an enclosing structure participates
 * in that structure's line breaking, rather than flattening into one
 * unbreakable token.
 *
 * That distinction only exists under a PrettySink -- ppsink's own tostr uses a
 * FlatSink and never breaks -- which is why this file depends on xo-indentlog2.
 */

#include "xo/alloc/Blob.hpp"
#include <xo/alloc/ArenaAlloc.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tostr.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace xo {
    using xo::gc::ArenaAlloc;
    using xo::pp::PrettySink;
    using xo::pp::PpConfig;

    namespace {
        /** render @p fn's output through a PrettySink with soft right margin
         *  @p margin, and return the rendered text.
         *
         *  NB the arena name must be unique per call: two PrettySinks sharing
         *  an ArenaConfig name interfere, and the symptom is wrong indentation
         *  in whichever case runs second.  (Same hazard documented in
         *  ObjectStatistics.test.cpp.)
         **/
        template <typename Fn>
        std::string
        render(std::uint32_t margin, Fn && fn) {
            static int seq = 0;

            PrettySink pps(PpConfig::scratch_plain(margin),
                           nullptr);

            fn(pps);

            return std::string(pps.output());
        }
    } /*namespace*/

    namespace ut {
        using xo::pp::field;

        TEST_CASE("object-pretty-composes", "[object_pretty]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            gp<Object> blob = gp<Object>::from(Blob::make(mm, 64));

            /* margin deliberately narrower than the flat rendering of the
             * outer struct, so something must break.
             */
            std::string s = render(8, [&blob](xo::pp::PpSink & sink) {
                    sink.pretty_struct("outer", field("b", blob));
                });

            INFO("rendered:\n" << s);

            /* Blob renders "<blob :z 64>".  If gp<Object> reaches the sink as
             * one atomic string token -- which is what happens when it falls
             * through Prettifier's empty primary template to operator<< --
             * then no break can appear inside it, however narrow the margin.
             *
             * So a newline between "<blob" and ":z" is exactly the evidence
             * that the object rendered STRUCTURALLY into the sink.
             */
            auto blob_at = s.find("<blob");
            auto z_at = s.find(":z");

            REQUIRE(blob_at != std::string::npos);
            REQUIRE(z_at != std::string::npos);
            REQUIRE(s.find('\n', blob_at) < z_at);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end object_pretty.test.cpp */
