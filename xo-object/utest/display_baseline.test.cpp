/* @file display_baseline.test.cpp
 *
 * author: Roland Conybeare, Aug 2026
 *
 * Pin the rendered form of every xo::gc::Object subclass in xo-object, ahead of
 * the display(std::ostream&) -> pretty(PpSink&) conversion
 * (.xo-backlog/xo-alloc/issues/01-object-pretty-ppsink.md).
 *
 * That conversion is meant to change no output at all, and this file is how
 * that claim is checked.  Nothing else pinned these renderings: before this
 * file, List was the only subclass in xo-object with any assertion on its
 * printed form (List.test.cpp, "List.display").
 *
 * Every expectation here was READ OFF observed output, not predicted.  Two
 * separate incidents on this migration came from writing expectations from
 * prediction -- see .xo-backlog/CONVENTIONS.md.
 */

#include "xo/object/Integer.hpp"
#include "xo/object/Float.hpp"
#include "xo/object/String.hpp"
#include "xo/object/Boolean.hpp"
#include "xo/object/List.hpp"
#include <xo/alloc/alloc_ostream.hpp>   /* os << gp<Object> */
#include <xo/alloc/ArenaAlloc.hpp>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>

namespace xo {
    using xo::gc::ArenaAlloc;
    using xo::obj::Integer;
    using xo::obj::Float;
    using xo::obj::String;
    using xo::obj::Boolean;
    using xo::obj::List;

    namespace ut {
        namespace {
            /** render @p x the way callers do today: through
             *  operator<<(std::ostream&, gp<Object>), which dispatches to
             *  Object::display().
             **/
            template <typename T>
            std::string
            rendered(gp<T> x) {
                std::stringstream ss;
                ss << x;
                return ss.str();
            }
        } /*namespace*/

        TEST_CASE("display-baseline-scalars", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            REQUIRE(rendered(Integer::make(mm, 0)) == "0");
            REQUIRE(rendered(Integer::make(mm, 123)) == "123");
            REQUIRE(rendered(Integer::make(mm, -321)) == "-321");

            REQUIRE(rendered(Boolean::true_obj()) == "#t");
            REQUIRE(rendered(Boolean::false_obj()) == "#f");
        }

        TEST_CASE("display-baseline-float", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* Float::display() is os << value_, so this inherits whatever
             * default formatting an ostream applies to the float type --
             * including how it renders a whole number.  Pinned so the
             * conversion cannot silently change it.
             */
            REQUIRE(rendered(Float::make(mm, 1.5)) == "1.5");
            REQUIRE(rendered(Float::make(mm, 0.0)) == "0");
            REQUIRE(rendered(Float::make(mm, -2.25)) == "-2.25");
        }

        TEST_CASE("display-baseline-string", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* String::display() is os << quot(c_str()) -- always quoted, with
             * xo::pp::Escape's rules inside.  Whether that is the right
             * contract for a Schematika string value is
             * .xo-backlog/xo-object/issues/01; this only pins what it does now.
             */
            REQUIRE(rendered(String::copy("abc")) == "\"abc\"");
            REQUIRE(rendered(String::copy("")) == "\"\"");
            REQUIRE(rendered(String::copy("a b")) == "\"a b\"");
            REQUIRE(rendered(String::copy("a\"b")) == "\"a\\\"b\"");

            /* tab is \t as of .xo-backlog/xo-ppsink/issues/04 (was \x09) */
            REQUIRE(rendered(String::copy("a\tb")) == "\"a\\tb\"");
        }

        TEST_CASE("display-baseline-list", "[display_baseline]") {
            constexpr bool c_debug_flag = false;

            up<ArenaAlloc> alloc = ArenaAlloc::make("arena", 4096, c_debug_flag);
            ArenaAlloc * mm = alloc.get();
            Object::mm = mm;

            /* List::display() renders each element with os << l->head(), i.e.
             * back through the same inserter.  So this also pins that nesting
             * keeps working -- the property the conversion has to preserve when
             * elements start going through Prettifier<gp<Object>> instead.
             */
            REQUIRE(rendered(List::nil) == "()");
            REQUIRE(rendered(List::list(Integer::make(mm, 1))) == "(1)");
            REQUIRE(rendered(List::list(Integer::make(mm, 1),
                                        Integer::make(mm, 2),
                                        Integer::make(mm, 3))) == "(1 2 3)");

            /* mixed element types, and a nested list: the recursive case */
            REQUIRE(rendered(List::list(Integer::make(mm, 1),
                                        String::copy("two"),
                                        Boolean::true_obj())) == "(1 \"two\" #t)");
            REQUIRE(rendered(List::list(Integer::make(mm, 1),
                                        List::list(Integer::make(mm, 2),
                                                   Integer::make(mm, 3))))
                    == "(1 (2 3))");
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end display_baseline.test.cpp */
