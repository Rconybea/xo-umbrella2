/** @file DArray.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/object2/DArray.hpp>
#include <xo/object2/DInteger.hpp>
#include <xo/object2/ListOps.hpp>
#include <xo/object2/number/IGCObject_DInteger.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::ListOps;
    using xo::scm::DArray;
    using xo::scm::DInteger;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        TEST_CASE("DArray-nullctor", "[object2][DArray]")
        {
            DArray arr;

            REQUIRE(arr.size() == 0);
            REQUIRE(arr.capacity() == 0);

            REQUIRE(arr.is_empty());;

            REQUIRE(arr.push_back(ListOps::nil()) == false);
        }

        TEST_CASE("DArray-empty", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 16);

            REQUIRE(arr != nullptr);
            REQUIRE(arr->capacity() == 16);

            REQUIRE(arr->is_empty() == true);
            REQUIRE(arr->size() == 0);
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
            REQUIRE(arr->capacity() == 16);
            REQUIRE(arr->size() == 0);

            obj<AGCObject> elt = DInteger::box<AGCObject>(alloc, 42);

            bool ok = arr->push_back(elt);

            REQUIRE(ok == true);
            REQUIRE(arr->is_empty() == false);
            REQUIRE(arr->size() == 1);
            REQUIRE(arr->capacity() == 16);
        }

        TEST_CASE("DArray-push_back-multiple", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 4);
            REQUIRE(arr != nullptr);
            REQUIRE(arr->capacity() == 4);
            REQUIRE(arr->size() == 0);

            for (int i = 0; i < 4; ++i) {
                REQUIRE(arr->capacity() == 4);
                REQUIRE(arr->size() == i);

                obj<AGCObject> elt = DInteger::box<AGCObject>(alloc, 100 + i);
                bool ok = arr->push_back(elt);
                REQUIRE(ok == true);

                REQUIRE(arr->capacity() == 4);
                REQUIRE(arr->is_empty() == false);
                REQUIRE(arr->size() == i+1);
            }
        }

        TEST_CASE("DArray-push_back-overflow", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 2);

            REQUIRE(arr != nullptr);
            REQUIRE(arr->capacity() == 2);
            REQUIRE(arr->size() == 0);

            obj<AGCObject> e1 = DInteger::box<AGCObject>(alloc, 1);
            obj<AGCObject> e2 = DInteger::box<AGCObject>(alloc, 2);
            obj<AGCObject> e3 = DInteger::box<AGCObject>(alloc, 3);

            REQUIRE(arr->push_back(e1) == true);
            REQUIRE(arr->size() == 1);
            REQUIRE(arr->push_back(e2) == true);
            REQUIRE(arr->size() == 2);
            REQUIRE(arr->push_back(e3) == false);
            REQUIRE(arr->size() == 2);
            REQUIRE(arr->capacity() == 2);
        }

        TEST_CASE("DArray-at", "[object2][DArray]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DArray * arr = DArray::empty(alloc, 4);

            REQUIRE(arr != nullptr);
            REQUIRE(arr->size() == 0);
            REQUIRE(arr->capacity() == 4);

            obj<AGCObject> e0 = DInteger::box<AGCObject>(alloc, 100);
            obj<AGCObject> e1 = DInteger::box<AGCObject>(alloc, 200);
            obj<AGCObject> e2 = DInteger::box<AGCObject>(alloc, 300);

            arr->push_back(e0);
            arr->push_back(e1);
            arr->push_back(e2);

            REQUIRE(arr->size() == 3);

            REQUIRE(arr->at(0).data() == e0.data());
            REQUIRE(arr->at(1).data() == e1.data());
            REQUIRE(arr->at(2).data() == e2.data());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DArray.test.cpp */
