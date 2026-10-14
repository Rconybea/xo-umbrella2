/* @file ReferenceTdx.test.cpp
 *
 * Reference types, reflected as mt_pointer: described, not traversed.
 * See .xo-backlog/std-function-reflection/spec.md, section 1b
 */

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/pointer/PointerRepr.hpp"
#include <xo/reflectutil/type_name.hpp>
#include <catch2/catch.hpp>
#include <memory>
#include <string>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;
    using xo::reflect::Metatype;
    using xo::reflect::PointerRepr;
    using xo::reflect::Ownership;

    namespace ut {
        TEST_CASE("lvalue-reference-reflects-as-pointer", "[reflect][reference]") {
            TypeDescr td = Reflect::require<std::string const &>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);
            REQUIRE(td->pointer_repr() == PointerRepr::lvalue_reference);
            /* the referent, cv-stripped -- shared with std::string * */
            REQUIRE(td->fixed_child_td(0) == Reflect::require<std::string>());
            REQUIRE(td->child_edge_ownership() == Ownership::borrowed);
            REQUIRE(td->canonical_name() == std::string(xo::reflect::type_name<std::string const &>()));
            /* typeid ignores references: no native typeinfo */
            REQUIRE(td->native_typeinfo() == nullptr);
        }

        TEST_CASE("rvalue-reference-reflects-as-pointer", "[reflect][reference]") {
            TypeDescr td = Reflect::require<std::string &&>();

            REQUIRE(td->metatype() == Metatype::mt_pointer);
            REQUIRE(td->pointer_repr() == PointerRepr::rvalue_reference);
            REQUIRE(td->fixed_child_td(0) == Reflect::require<std::string>());
        }

        /* one entry per reference type; never its referent's, never a
         * pointer's -- typeid(T&) == typeid(T)
         */
        TEST_CASE("reference-type-is-its-own-entry", "[reflect][reference]") {
            TypeDescr value_td = Reflect::require<std::string>();
            TypeDescr ref_td = Reflect::require<std::string const &>();
            TypeDescr mref_td = Reflect::require<std::string &>();
            TypeDescr ptr_td = Reflect::require<std::string *>();

            REQUIRE(Reflect::require<std::string const &>() == ref_td);
            REQUIRE(ref_td != value_td);
            REQUIRE(ref_td != mref_td);
            REQUIRE(ref_td != ptr_td);
            /* the referent is unchanged: still found by its own typeid */
            REQUIRE(Reflect::require<std::string>() == value_td);
            REQUIRE(value_td->metatype() != Metatype::mt_pointer);
            REQUIRE(value_td->native_typeinfo() == &typeid(std::string));
        }

        TEST_CASE("pointer-repr-of-each-pointer-kind", "[reflect][reference]") {
            REQUIRE(Reflect::require<int *>()->pointer_repr() == PointerRepr::raw);
            REQUIRE(Reflect::require<std::unique_ptr<int>>()->pointer_repr() == PointerRepr::smart);
            REQUIRE(Reflect::require<int &>()->pointer_repr() == PointerRepr::lvalue_reference);
        }

        /* type-level only: a reference is not an object to traverse */
        TEST_CASE("reference-value-operations-throw", "[reflect][reference]") {
            TypeDescr td = Reflect::require<int &>();
            int x = 0;

            REQUIRE_THROWS(td->n_child(&x));
            REQUIRE_THROWS(td->child_tp(0, &x));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end ReferenceTdx.test.cpp */
