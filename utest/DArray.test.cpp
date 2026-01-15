/** @file DArray.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/object2/DArray.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::DArray;
    using xo::scm::DInteger;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        TEST_CASE("DArray-empty", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 16);

            REQUIRE(arr != nullptr);
            REQUIRE(arr->is_empty() == true);
            REQUIRE(arr->is_finite() == true);
        }

        TEST_CASE("DArray-push_back", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 16);
            REQUIRE(arr != nullptr);

            obj<AGCObject> elt = DInteger::box<AGCObject>(alloc, 42);

            bool ok = arr->push_back(elt);

            REQUIRE(ok == true);
            REQUIRE(arr->is_empty() == false);
        }

        TEST_CASE("DArray-push_back-multiple", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 4);
            REQUIRE(arr != nullptr);

            for (int i = 0; i < 4; ++i) {
                obj<AGCObject> elt = DInteger::box<AGCObject>(alloc, 100 + i);
                bool ok = arr->push_back(elt);
                REQUIRE(ok == true);
            }

            REQUIRE(arr->is_empty() == false);
        }

        TEST_CASE("DArray-push_back-overflow", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 2);
            REQUIRE(arr != nullptr);

            obj<AGCObject> e1 = DInteger::box<AGCObject>(alloc, 1);
            obj<AGCObject> e2 = DInteger::box<AGCObject>(alloc, 2);
            obj<AGCObject> e3 = DInteger::box<AGCObject>(alloc, 3);

            REQUIRE(arr->push_back(e1) == true);
            REQUIRE(arr->push_back(e2) == true);
            REQUIRE(arr->push_back(e3) == false);
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DArray.test.cpp */
