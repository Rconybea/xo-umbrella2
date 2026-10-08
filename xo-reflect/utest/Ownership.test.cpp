/** @file Ownership.test.cpp
 *
 *  ownership edges -- see .xo-backlog/xo-reflect/issues/07.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/StructReflector.hpp"
#include "xo/reflect/Reflect.hpp"
#include <xo/refcnt/Refcounted.hpp>
#include <catch2/catch.hpp>
#include <memory>
#include <vector>

namespace xo {
    using xo::reflect::Ownership;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TypeDescr;

    namespace ut {
        namespace {
            struct Leaf { int x_ = 0; };

            struct Counted : public ref::Refcount { int n_ = 0; };

            /* one member of each pointer kind, two of them overridden */
            struct Holder {
                Leaf leaf_;
                Leaf * raw_ = nullptr;
                rp<Counted> shared_;
                std::unique_ptr<Leaf> owned_;
                Leaf * raw_owned_ = nullptr;
                rp<Counted> shared_borrowed_;
            };

            struct DerivedHolder : public Holder {};

            void reflect_holder() {
                StructReflector<Holder> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, leaf);
                    REFLECT_MEMBER(sr, raw);
                    REFLECT_MEMBER(sr, shared);
                    REFLECT_MEMBER(sr, owned);
                    REFLECT_MEMBER(sr, raw_owned).owning();
                    REFLECT_MEMBER(sr, shared_borrowed).borrowed();
                }
            }

            void reflect_derived_holder() {
                reflect_holder();

                StructReflector<DerivedHolder> sr;

                if (sr.is_incomplete())
                    sr.adopt_ancestors<Holder>();
            }

            /* ownership of member @p name of the reflected struct @p td */
            Ownership member_ownership(TypeDescr td, std::string const & name) {
                for (uint32_t i = 0, n = td->n_child(nullptr); i < n; ++i) {
                    if (td->struct_member(i).member_name() == name)
                        return td->struct_member(i).ownership();
                }

                FAIL("no member " << name);
                return Ownership::owning;
            }
        } /*namespace*/

        TEST_CASE("ownership-type-defaults", "[reflect][ownership]") {
            /* inline storage owns its children */
            REQUIRE(Reflect::require<int>()->child_edge_ownership() == Ownership::owning);
            REQUIRE(Reflect::require<std::vector<int>>()->child_edge_ownership() == Ownership::owning);

            /* pointers report their pointee's edge */
            REQUIRE(Reflect::require<Leaf *>()->child_edge_ownership() == Ownership::borrowed);
            REQUIRE(Reflect::require<rp<Counted>>()->child_edge_ownership() == Ownership::shared);
            REQUIRE(Reflect::require<std::unique_ptr<Leaf>>()->child_edge_ownership() == Ownership::owning);
        }

        TEST_CASE("ownership-composes-through-containers", "[reflect][ownership]") {
            /* a vector owns its (pointer) elements; each element borrows */
            TypeDescr v = Reflect::require<std::vector<Leaf *>>();

            REQUIRE(v->child_edge_ownership() == Ownership::owning);
            REQUIRE(v->fixed_child_td(0)->child_edge_ownership() == Ownership::borrowed);
        }

        TEST_CASE("ownership-member-defaults-and-overrides", "[reflect][ownership]") {
            reflect_holder();

            TypeDescr td = Reflect::require<Holder>();

            /* defaults, from the member type */
            REQUIRE(member_ownership(td, "leaf") == Ownership::owning);
            REQUIRE(member_ownership(td, "raw") == Ownership::borrowed);
            REQUIRE(member_ownership(td, "shared") == Ownership::shared);
            REQUIRE(member_ownership(td, "owned") == Ownership::owning);

            /* overrides win */
            REQUIRE(member_ownership(td, "raw_owned") == Ownership::owning);
            REQUIRE(member_ownership(td, "shared_borrowed") == Ownership::borrowed);
        }

        TEST_CASE("ownership-override-survives-adopt-ancestors", "[reflect][ownership]") {
            reflect_derived_holder();

            TypeDescr td = Reflect::require<DerivedHolder>();

            REQUIRE(member_ownership(td, "raw") == Ownership::borrowed);
            REQUIRE(member_ownership(td, "raw_owned") == Ownership::owning);
            REQUIRE(member_ownership(td, "shared_borrowed") == Ownership::borrowed);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end Ownership.test.cpp */
