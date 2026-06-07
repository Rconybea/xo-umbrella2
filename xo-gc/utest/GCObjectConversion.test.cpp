/** @file GCObjectConversion.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include "GCObjectConversion.hpp"
#include <xo/object2/ListOps.hpp>
#include <xo/object2/List.hpp>
#include <xo/object2/Array.hpp>
#include <xo/object2/Integer.hpp>
#include <xo/alloc2/Arena.hpp>
#include <catch2/catch.hpp>

namespace xo {
    //using xo::scm::ASequence;
    using xo::scm::ListOps;
    using xo::scm::DArray;
    using xo::scm::DList;
    using xo::scm::DInteger;
    using xo::scm::GCObjectConversion;
    using xo::mm::AGCObject;
    using xo::mm::ArenaConfig;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::facet::obj;

    namespace ut {

        TEST_CASE("GCObjectConversion-1", "[GCObjectConversion]")
        {
            scope log(XO_DEBUG(true), "GCObjectConversion-1");

            ArenaConfig cfg {
                .name_ = "testarena",
                .size_ = 128
            };
            DArena arena = DArena::map(cfg);
            auto mm = obj<AAllocator,DArena>(&arena);
            auto v1 = DArray::empty(mm, 3);

            REQUIRE(v1);
            REQUIRE(v1->size() == 0);

            {
                obj v1_seq
                    = GCObjectConversion<obj<AGCObject,DArray>>::from_gco(mm /*not used*/, v1);

                REQUIRE(v1_seq);
                REQUIRE(v1_seq == v1);
                REQUIRE(v1_seq->size() == 0);
            }

            {
                obj l1_seq
                    = GCObjectConversion<obj<AGCObject,DList>>::from_gco(mm /*not used*/, v1);

                REQUIRE(!l1_seq);
            }
        }

        TEST_CASE("GCObjectConversion-2", "[GCObjectConversion]")
        {
            scope log(XO_DEBUG(true), "GCObjectConversion-2");

            ArenaConfig cfg {
                .name_ = "testarena",
                .size_ = 128
            };
            DArena arena = DArena::map(cfg);
            auto mm = obj<AAllocator,DArena>(&arena);
            auto l1 = ListOps::cons(mm, DInteger::box(mm, 42), ListOps::nil());

            REQUIRE(l1);
            REQUIRE(l1->size() == 1);

            {
                // will fail; source is DArena
                obj l1_seq
                    = GCObjectConversion<obj<AGCObject,DList>>::from_gco(mm /*not used*/, l1);

                REQUIRE(l1_seq);
            }

            {
                obj v1_seq
                    = GCObjectConversion<obj<AGCObject,DArray>>::from_gco(mm /*not used*/, l1);

                REQUIRE(!v1_seq);
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end GCObjectConversion.cpp */
