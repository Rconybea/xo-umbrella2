/* @file display_baseline.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Pin the rendered form of xo-alloc's xo::gc::Object subclasses, ahead of the
 * display(std::ostream&) -> pretty(PpSink&) conversion
 * (.xo-backlog/xo-alloc/issues/01-object-pretty-ppsink.md).
 *
 * The conversion is meant to change no output; this is how that is checked.
 * Companion file: xo-object/utest/display_baseline.test.cpp.
 *
 * Expectations here were read off observed output, not predicted -- see
 * .xo-backlog/CONVENTIONS.md on measuring before asserting.
 */

#include "xo/alloc/Blob.hpp"
#include "xo/alloc/Forwarding1.hpp"
#include "xo/alloc/alloc_ostream.hpp"   /* os << gp<Object> */
#include <xo/alloc/ArenaAlloc.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::gc::ArenaAlloc;
    using xo::obj::Forwarding1;
    /* NB Blob and Object live directly in namespace xo, not xo::gc --
     * only the allocator/statistics types are in xo::gc.
     */

    namespace ut {
        namespace {
            template <typename T>
            std::string
            rendered(gp<T> x) {
                std::stringstream ss;
                ss << gp<Object>::from(x);
                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("display-baseline-blob", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* Blob::display() is: os << "<blob" << xtag("z", z_) << ">".
             * The xtag spelling is what makes this worth pinning -- the
             * conversion has to keep ppsink's tag rendering identical, not
             * merely produce "some tag".
             */
            REQUIRE(rendered(Blob::make(mm, 0)) == "<blob :z 0>");
            REQUIRE(rendered(Blob::make(mm, 64)) == "<blob :z 64>");
        }

        TEST_CASE("display-baseline-forwarding1", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            gp<Blob> target = Blob::make(mm, 8);
            gp<Forwarding1> fwd = new Forwarding1(gp<Object>::from(target));

            /* Forwarding1::display() embeds the destination pointer, so the
             * exact bytes vary per run.  Pin the shape instead: the conversion
             * must keep the "<fwd :dest 0x..>" framing, which is what a reader
             * of GC diagnostics actually relies on.
             */
            std::string s = rendered(fwd);

            INFO("rendered: " << s);
            REQUIRE(s.starts_with("<fwd :dest "));
            REQUIRE(s.ends_with(">"));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end display_baseline.test.cpp */
