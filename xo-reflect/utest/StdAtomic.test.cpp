/** @file StdAtomic.test.cpp
 *
 *  reflection for std::atomic<T> -- see .xo-backlog/xo-reflect/issues/04.
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/EnumReflector.hpp"
#include <catch2/catch.hpp>
#include <atomic>
#include <cstdint>
#include <cstring>

namespace xo {
    using xo::reflect::EnumReflector;
    using xo::reflect::Metatype;
    using xo::reflect::Reflect;
    using xo::reflect::StdAtomicTdx;
    using xo::reflect::TypeDescr;

    namespace ut {
        namespace {
            enum class Phase { idle, busy };

            void reflect_phase() {
                EnumReflector<Phase> er;

                if (er.is_incomplete()) {
                    REFLECT_ENUM(er, idle);
                    REFLECT_ENUM(er, busy);
                }
            }

            /** load @p a through reflection, into a T **/
            template <typename T>
            T load_reflected(std::atomic<T> const & a) {
                StdAtomicTdx const * ai = Reflect::require<std::atomic<T>>()->std_atomic_info();
                REQUIRE(ai != nullptr);
                REQUIRE(ai->value_size() == sizeof(T));
                REQUIRE(ai->value_align() == alignof(T));

                alignas(T) unsigned char buf[sizeof(T)];
                ai->load(&a, buf);

                T retval;
                std::memcpy(&retval, buf, sizeof(T));
                return retval;
            }
        } /*namespace*/

        TEST_CASE("std-atomic-reflects-as-an-atomic-with-a-load", "[reflect][stdatomic]") {
            TypeDescr td = Reflect::require<std::atomic<int>>();

            /* not traversable in place: no children */
            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(td->n_child_fixed() == 0);
            REQUIRE(td->is_std_atomic());
            REQUIRE(!td->is_enum());

            /* its value: an int */
            REQUIRE(td->std_atomic_info()->value_td() == Reflect::require<int>());
        }

        TEST_CASE("std-atomic-load-copies-the-current-value", "[reflect][stdatomic]") {
            std::atomic<bool> b{true};
            std::atomic<std::int64_t> i{-42};
            std::atomic<double> d{2.5};

            REQUIRE(load_reflected(b) == true);
            REQUIRE(load_reflected(i) == -42);
            REQUIRE(load_reflected(d) == 2.5);

            /* a later store: a later load sees it */
            i.store(7);
            REQUIRE(load_reflected(i) == 7);
        }

        TEST_CASE("std-atomic-of-a-reflected-enum", "[reflect][stdatomic]") {
            /* the loaded value is reflected as its own type -- here an enum,
             * so a consumer can name it
             */
            reflect_phase();

            std::atomic<Phase> p{Phase::busy};

            StdAtomicTdx const * ai = Reflect::require<std::atomic<Phase>>()->std_atomic_info();
            REQUIRE(ai->value_td() == Reflect::require<Phase>());
            REQUIRE(ai->value_td()->is_enum());
            REQUIRE(load_reflected(p) == Phase::busy);
        }

        TEST_CASE("plain-scalar-is-not-a-std-atomic", "[reflect][stdatomic]") {
            TypeDescr td = Reflect::require<int>();

            REQUIRE(td->metatype() == Metatype::mt_atomic);
            REQUIRE(!td->is_std_atomic());
            REQUIRE(td->std_atomic_info() == nullptr);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end StdAtomic.test.cpp */
