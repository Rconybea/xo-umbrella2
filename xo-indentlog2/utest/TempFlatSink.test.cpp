/* @file TempFlatSink.test.cpp
 *
 * author: Roland Conybeare, Oct 2026
 *
 * xo::pp::TempFlatSink -- a FlatSink over per-thread scratch memory
 * (TempArena), whose text is read back afterwards.
 */

#include "print/TempFlatSink.hpp"
#include "print/tostr.hpp"
#include "xo/indentlog2/TempArena.hpp"
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/quoted.hpp>
#include <catch2/catch.hpp>
#include <string>
#include <string_view>

namespace xo {
    namespace ut {
        using xo::pp::TempFlatSink;
        using xo::pp::PpStyle;
        using xo::pp::quot;
        using xo::pp::tostr;
        using xo::mm::TempArena;

        TEST_CASE("temp-flat-sink-collects-output", "[TempFlatSink]") {
            TempFlatSink out(PpStyle::plain());

            out.sink().put("a");
            out.sink().pp(42);
            out.sink().put("b");

            REQUIRE(out.output() == "a42b");
        }

        /* the sink's style is the one asked for */
        TEST_CASE("temp-flat-sink-uses-its-style", "[TempFlatSink]") {
            TempFlatSink json(PpStyle::json());
            json.sink().pp(quot(std::string_view("a\0b", 3)));

            TempFlatSink display(PpStyle::plain());
            display.sink().pp(quot(std::string_view("a\0b", 3)));

            REQUIRE(json.output() == "\"a\\u0000b\"");
            REQUIRE(display.output() == "\"a\\x00b\"");
        }

        /* a tostr() -- also on TempArena -- run and finished while the sink
         * is alive, as printjson's map_key_string does mid-print, leaves the
         * sink's text intact, before and after
         */
        TEST_CASE("temp-flat-sink-survives-nested-tostr", "[TempFlatSink]") {
            TempFlatSink out(PpStyle::plain());

            out.sink().put("before:");
            std::string inner = tostr("inner", 1, 2, 3);
            out.sink().put(inner);
            out.sink().put(":after");

            /* enough to make the outer buffer grow past its first allocation */
            std::string big(10000, 'x');
            out.sink().put(big);

            REQUIRE(inner == "inner123");
            REQUIRE(out.output() == "before:inner123:after" + big);
        }

        /* scratch memory goes back when the sink does */
        TEST_CASE("temp-flat-sink-restores-arena", "[TempFlatSink]") {
            auto allocated0 = TempArena::local().allocated();
            {
                TempFlatSink out(PpStyle::plain());
                out.sink().put(std::string(5000, 'y'));
                REQUIRE(out.output().size() == 5000);
            }
            REQUIRE(TempArena::local().allocated() == allocated0);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end TempFlatSink.test.cpp */
