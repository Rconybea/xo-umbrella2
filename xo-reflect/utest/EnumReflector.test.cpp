/** @file EnumReflector.test.cpp
 *
 *  reflection for enums -- see .xo-backlog/xo-reflect/issues/06.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/EnumReflector.hpp"
#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>

namespace xo {
    using xo::reflect::EnumReflector;
    using xo::reflect::EnumTdx;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace ut {
        namespace {
            /* scoped; values given out of order, and one shared */
            enum class Color : std::uint8_t { red = 2, green = 5, blue = 1, crimson = 2 };

            /* unscoped */
            enum Shape { circle, square, triangle };

            /* never reflected */
            enum class Unreflected { a, b };

            void reflect_color() {
                EnumReflector<Color> er;

                if (er.is_incomplete()) {
                    REFLECT_ENUM(er, red);
                    REFLECT_ENUM(er, green);
                    REFLECT_ENUM(er, blue);
                    REFLECT_ENUM(er, crimson);
                }
            }

            void reflect_shape() {
                EnumReflector<Shape> er;

                if (er.is_incomplete()) {
                    REFLECT_ENUM(er, circle);
                    REFLECT_EXPLICIT_ENUM(er, "Square", square);
                    REFLECT_ENUM(er, triangle);
                }
            }
        } /*namespace*/

        TEST_CASE("enum-reflects-as-an-atomic-with-enumerators", "[reflect][enum]") {
            reflect_color();

            TypeDescr td = Reflect::require<Color>();

            /* an enum composes no other type */
            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(td->n_child_fixed() == 0);
            REQUIRE(td->is_enum());

            EnumTdx const * ei = td->enum_info();
            REQUIRE(ei != nullptr);

            /* declaration order, with their values */
            REQUIRE(ei->n_enumerator() == 4);
            REQUIRE(ei->enumerator_name(0) == "red");
            REQUIRE(ei->enumerator_value(0) == 2);
            REQUIRE(ei->enumerator_name(1) == "green");
            REQUIRE(ei->enumerator_value(1) == 5);
            REQUIRE(ei->enumerator_name(2) == "blue");
            REQUIRE(ei->enumerator_value(2) == 1);
            REQUIRE(ei->enumerator_name(3) == "crimson");
            REQUIRE(ei->enumerator_value(3) == 2);
        }

        TEST_CASE("enum-value-to-name-and-back", "[reflect][enum]") {
            reflect_color();

            EnumTdx const * ei = Reflect::require<Color>()->enum_info();
            REQUIRE(ei != nullptr);

            Color c = Color::green;

            REQUIRE(ei->value_of(&c) == 5);
            REQUIRE(ei->name_of(&c) != nullptr);
            REQUIRE(*ei->name_of(&c) == "green");

            /* a shared value: the first enumerator with it */
            c = Color::crimson;
            REQUIRE(*ei->name_of(&c) == "red");

            REQUIRE(ei->assign_from_name("blue", &c));
            REQUIRE(c == Color::blue);
            REQUIRE(ei->assign_from_name("crimson", &c));
            REQUIRE(c == Color::red);
        }

        TEST_CASE("enum-value-with-no-enumerator", "[reflect][enum]") {
            reflect_color();

            EnumTdx const * ei = Reflect::require<Color>()->enum_info();

            /* not a reflected value: no name, but its integer */
            Color c = static_cast<Color>(7);

            REQUIRE(ei->name_of(&c) == nullptr);
            REQUIRE(ei->value_of(&c) == 7);

            /* an unknown name: false, the object unchanged */
            c = Color::green;
            REQUIRE(!ei->assign_from_name("mauve", &c));
            REQUIRE(c == Color::green);
        }

        TEST_CASE("enum-unscoped-and-explicit-names", "[reflect][enum]") {
            reflect_shape();

            EnumTdx const * ei = Reflect::require<Shape>()->enum_info();
            REQUIRE(ei != nullptr);
            REQUIRE(ei->n_enumerator() == 3);

            Shape s = square;
            REQUIRE(*ei->name_of(&s) == "Square");

            REQUIRE(ei->assign_from_name("triangle", &s));
            REQUIRE(s == triangle);
        }

        TEST_CASE("enum-reflected-once", "[reflect][enum]") {
            /* a second reflector adds nothing: completion is once per type */
            reflect_color();
            reflect_color();

            REQUIRE(Reflect::require<Color>()->enum_info()->n_enumerator() == 4);
        }

        TEST_CASE("enum-not-reflected-is-a-plain-atomic", "[reflect][enum]") {
            TypeDescr td = Reflect::require<Unreflected>();

            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(!td->is_enum());
            REQUIRE(td->enum_info() == nullptr);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end EnumReflector.test.cpp */
