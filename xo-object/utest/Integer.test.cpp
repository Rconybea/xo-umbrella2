/* @file Integer.test.cpp
 *
 * author Roland Conybeare, Aug 2025
 */

#include "xo/object/Integer.hpp"
#include "xo/alloc/GC.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::gc::GC;
    using xo::gc::generation_result;
    using xo::obj::Integer;

    namespace ut {
        TEST_CASE("Integer", "[Integer]")
        {
            up<GC> gc = GC::make(
                { .initial_nursery_z_ = 1024,
                  .initial_tenured_z_ = 1024
                });

            REQUIRE(gc.get());

            Object::mm = gc.get();

            gp<Integer> i1 = Integer::make(123);
            gp<Integer> i2 = Integer::make(-321);

            REQUIRE(i1->value() == 123);
            REQUIRE(i2->value() == -321);

            REQUIRE(i1->_shallow_size() == sizeof(Integer));
            REQUIRE(i2->_shallow_size() == sizeof(Integer));

            REQUIRE(gc->tospace_generation_of(i1.ptr()) == generation_result::nursery);
            REQUIRE(gc->tospace_generation_of(i2.ptr()) == generation_result::nursery);

            REQUIRE(i1->self_tp().td()->short_name() == "Integer");
            REQUIRE(i2->self_tp().td()->short_name() == "Integer");

            {
                std::stringstream ss;
                ss << i1;
                REQUIRE(ss.str() == "123");
            }

            {
                std::stringstream ss;
                ss << i2;
                REQUIRE(ss.str() == "-321");
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* Integer.test.cpp */
