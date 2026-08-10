/* @file object_pretty.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * The capability the display() -> pretty(PpSink&) conversion adds, which flat
 * output cannot show: a List breaks across lines when it does not fit, and its
 * elements nest structurally instead of flattening into one token.
 *
 * display_baseline.test.cpp is the other half -- it pins that flat rendering
 * did not change.  Both must hold.
 *
 * Needs a PrettySink (xo-indentlog2): ppsink's own tostr uses a FlatSink and
 * never breaks.
 */

#include "xo/object/Integer.hpp"
#include "xo/object/List.hpp"
#include <xo/alloc/ArenaAlloc.hpp>
#include <xo/indentlog2/print/toppstr.hpp>
#include <xo/ppsink/pretty.hpp>
#include "print/PrettySink.hpp"
#include <xo/arena/ArenaConfig.hpp>
#include <catch2/catch.hpp>
#include <string>

namespace xo {
    using xo::gc::ArenaAlloc;
    using xo::obj::Integer;
    using xo::obj::List;
    using xo::pp::PpConfig;
    using xo::pp::toppstr;

    namespace {
        /** render @p x through a PrettySink with soft right margin @p margin.
         *
         *  NB the arena name must be unique per call: two PrettySinks sharing
         *  an ArenaConfig name interfere, and the symptom is wrong indentation
         *  in whichever case runs second.
         **/
        template <typename T>
        std::string
        render(std::uint32_t margin, const T & x) {
            return toppstr(PpConfig::scratch_plain(margin), x);
        }
    } /*namespace*/

    namespace ut {
        TEST_CASE("list-pretty-breaks-when-long", "[object_pretty]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 16384, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            gp<List> l = List::list(Integer::make(mm, 111),
                                    Integer::make(mm, 222),
                                    Integer::make(mm, 333),
                                    Integer::make(mm, 444));

            gp<Object> obj = gp<Object>::from(l);

            /* wide margin: fits, so no break -- and byte-identical to the flat
             * form display() produced.
             */
            REQUIRE(render(80, obj) == "(111 222 333 444)");

            /* narrow margin: the separators between elements are split(1)
             * rather than a literal space, so they can become line breaks.
             * Before the conversion this rendering was one atomic token and
             * could not break at any margin.
             */
            std::string narrow = render(8, obj);

            INFO("narrow:\n" << narrow);
            REQUIRE(narrow.find('\n') != std::string::npos);

            /* breaking must not lose or duplicate elements */
            for (const char * elt : {"111", "222", "333", "444"})
                REQUIRE(narrow.find(elt) != std::string::npos);
        }

        TEST_CASE("list-pretty-nests", "[object_pretty]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 16384, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* a nested list reaches the sink via Prettifier<gp<Object>>, so it
             * renders structurally -- an inner list is itself breakable, not a
             * pre-flattened string.
             */
            gp<Object> inner = gp<Object>::from(List::list(Integer::make(mm, 111),
                                                           Integer::make(mm, 222)));
            gp<Object> outer = gp<Object>::from(List::list(Integer::make(mm, 1), inner));

            REQUIRE(render(80, outer) == "(1 (111 222))");

            std::string narrow = render(6, outer);

            INFO("narrow:\n" << narrow);
            REQUIRE(narrow.find('\n') != std::string::npos);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end object_pretty.test.cpp */
