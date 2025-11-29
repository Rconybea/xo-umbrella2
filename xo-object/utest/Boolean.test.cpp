/* @file Boolean.test.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/Boolean.hpp"
#include "xo/alloc/GC.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::obj::Boolean;
    using xo::gc::GC;
    using xo::gc::generation_result;

    namespace ut {
        TEST_CASE("Boolean", "[Boolean]")
        {
            up<GC> gc = GC::make(
                { .initial_nursery_z_ = 1024,
                  .initial_tenured_z_ = 2048,
                  .incr_gc_threshold_ = 512,
                  .full_gc_threshold_ = 512
                });

            REQUIRE(gc.get());

            /* use gc for "all" Object allocs.
             * Not using, but want it to be available, verify conscious choice
             */
            Object::mm = gc.get();

            gp<Boolean> btrue = Boolean::true_obj();
            gp<Boolean> bfalse = Boolean::false_obj();

            REQUIRE(btrue.ptr());
            REQUIRE(btrue->value() == true);

            REQUIRE(bfalse.ptr());
            REQUIRE(bfalse->value() == false);

            REQUIRE(btrue->_shallow_size() == sizeof(Boolean));

            // booleans are global constants
            REQUIRE(gc->tospace_generation_of(btrue.ptr()) == generation_result::not_found);
            REQUIRE(gc->tospace_generation_of(bfalse.ptr()) == generation_result::not_found);

            REQUIRE(btrue->self_tp().td()->short_name() == "Boolean");
            REQUIRE(bfalse->self_tp().td()->short_name() == "Boolean");

            {
                std::stringstream ss;
                ss << btrue;
                REQUIRE(ss.str() == "#t");
            }

            {
                std::stringstream ss;
                ss << bfalse;
                REQUIRE(ss.str() == "#f");
            }
        }
    } /*namespace ut*/
} /*namespace xo*/
