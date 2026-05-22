/** @file dp.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include "dp.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;

    namespace {
        class Foo {
        public:
            explicit Foo(uint32_t * p_counter) : p_counter_{p_counter} {}

            static constexpr bool is_gc_eligible() { return false; }

            ~Foo() {
                ++(*p_counter_);
            }

        private:
            uint32_t * p_counter_ = nullptr;
        };
    }

    namespace ut {

        TEST_CASE("dp-1", "[dp]")
        {
            auto log = Utest::ut_scope();

            //ArenaConfig cfg { .name_ = "testarena", .size_ = 1024 };
            //DArena arena = DArena::map(cfg);
            //auto mm = obj<AAllocator,DArena>(&arena);

            uint32_t counter = 0;
            Foo foo(&counter);

            REQUIRE(counter == 0);
            {
                dp<Foo> foo_dp(&foo);

                REQUIRE(foo_dp);
                REQUIRE(foo_dp.data());
                REQUIRE(foo_dp.is_gc_eligible() == false);

                // foo_dp dtor runs here, increments counter
            }
            REQUIRE(counter == 1);
        }

        TEST_CASE("dp-2", "[dp]")
        {
            auto log = Utest::ut_scope();

            uint32_t counter = 0;
            Foo foo(&counter);

            REQUIRE(counter == 0);
            {
                dp<Foo> foo_dp(&foo);

                REQUIRE(foo_dp);
                REQUIRE(foo_dp.data() == &foo);

                foo_dp.release();

                REQUIRE(!foo_dp);
                REQUIRE(!foo_dp.data());

                // foo_dp dtor runs here, increments counter
            }
            REQUIRE(counter == 0);
        }

        TEST_CASE("dp-DArena", "[dp][DArena]")
        {
            auto log = Utest::ut_scope();

            ArenaConfig cfg { .name_ = "testarena", .size_ = 1024 };
            DArena arena = DArena::map(cfg);
            //auto mm = obj<AAllocator,DArena>(&arena);

            REQUIRE(arena.reserved() > 0);
            REQUIRE(arena.is_mapped());
            {
                dp<DArena> arena_dp(&arena);

                REQUIRE(arena_dp);
                REQUIRE(arena_dp.data() == &arena);
            }

            REQUIRE(arena.reserved() == 0);
            REQUIRE(!arena.is_mapped());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end dp.test.cpp */
