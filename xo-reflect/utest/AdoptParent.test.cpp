/** @file AdoptParent.test.cpp
 *
 *  declared parents and TypeDescr::is_derived_from -- see
 *  .xo-backlog/xo-printjson/issues/08 (borrowed refs match a placed object
 *  of the pointee's type or one derived from it).
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/StructReflector.hpp"
#include "xo/reflect/Reflect.hpp"
#include <catch2/catch.hpp>
#include <string>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TypeDescr;

    namespace ut {
        namespace {
            struct Animal { int legs_ = 0; };
            struct Dog : public Animal { int bark_ = 0; };
            struct Puppy : public Dog { int age_ = 0; };

            /* two bases: multiple inheritance */
            struct Pet { int name_len_ = 0; };
            struct PetDog : public Dog, public Pet {};

            struct Rock { int mass_ = 0; };

            void reflect_animals() {
                {
                    StructReflector<Animal> sr;
                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, legs);
                }
                {
                    StructReflector<Dog> sr;
                    if (sr.is_incomplete()) {
                        sr.adopt_parent<Animal>();
                        REFLECT_MEMBER(sr, bark);
                    }
                }
                {
                    StructReflector<Puppy> sr;
                    if (sr.is_incomplete()) {
                        sr.adopt_parent<Dog>();
                        REFLECT_MEMBER(sr, age);
                    }
                }
                {
                    StructReflector<Pet> sr;
                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, name_len);
                }
                {
                    StructReflector<PetDog> sr;
                    if (sr.is_incomplete()) {
                        sr.adopt_parent<Dog>();
                        sr.adopt_parent<Pet>();
                    }
                }
                {
                    StructReflector<Rock> sr;
                    if (sr.is_incomplete())
                        REFLECT_MEMBER(sr, mass);
                }
            }
        } /*namespace*/

        TEST_CASE("adopt-parent-records-parents", "[reflect][parent]") {
            reflect_animals();

            TypeDescr dog = Reflect::require<Dog>();
            TypeDescr pet_dog = Reflect::require<PetDog>();

            REQUIRE(Reflect::require<Animal>()->n_parent() == 0);
            REQUIRE(dog->n_parent() == 1);
            REQUIRE(dog->parent_td(0) == Reflect::require<Animal>());
            REQUIRE(pet_dog->n_parent() == 2);
            REQUIRE(pet_dog->parent_td(0) == dog);
            REQUIRE(pet_dog->parent_td(1) == Reflect::require<Pet>());

            /* and adopts members, as before: Animal's, then its own */
            REQUIRE(dog->n_child(nullptr) == 2);
            REQUIRE(dog->struct_member(0).member_name() == "legs");
            REQUIRE(dog->struct_member(1).member_name() == "bark");
        }

        TEST_CASE("is-derived-from", "[reflect][parent]") {
            reflect_animals();

            TypeDescr animal = Reflect::require<Animal>();
            TypeDescr dog = Reflect::require<Dog>();
            TypeDescr puppy = Reflect::require<Puppy>();
            TypeDescr pet = Reflect::require<Pet>();
            TypeDescr pet_dog = Reflect::require<PetDog>();
            TypeDescr rock = Reflect::require<Rock>();

            /* itself */
            REQUIRE(dog->is_derived_from(dog));
            /* direct, and transitive */
            REQUIRE(dog->is_derived_from(animal));
            REQUIRE(puppy->is_derived_from(animal));
            /* through either base */
            REQUIRE(pet_dog->is_derived_from(animal));
            REQUIRE(pet_dog->is_derived_from(pet));

            /* not upward, not sideways */
            REQUIRE(!animal->is_derived_from(dog));
            REQUIRE(!dog->is_derived_from(puppy));
            REQUIRE(!dog->is_derived_from(pet));
            REQUIRE(!rock->is_derived_from(animal));
            /* non-structs have no parents */
            REQUIRE(!Reflect::require<int>()->is_derived_from(animal));
            REQUIRE(Reflect::require<int>()->is_derived_from(Reflect::require<int>()));
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end AdoptParent.test.cpp */
