/** @file DListType.test.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "init_type.hpp"
#include "TypeOps.hpp"
#include "ListType.hpp"
#include "AtomicType.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/obj.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::TypeOps;
    using xo::scm::AType;
    //using xo::scm::DListType;
    //using xo::scm::DAtomicType;
    using xo::scm::Metatype;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_type_tag>::require());

        TEST_CASE("DListType-make", "[type][DListType]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = obj<AAllocator,DArena>(&arena);

            auto i64_type = TypeOps::atomic_type(alloc, Metatype::t_i64());
            auto list_i64_type = TypeOps::list_type(alloc, i64_type);

            REQUIRE(list_i64_type);
            REQUIRE(list_i64_type.is_equal_to(list_i64_type));

            auto bool_type = TypeOps::atomic_type(alloc, Metatype::t_bool());
            auto list_bool_type = TypeOps::list_type(alloc, bool_type);

            REQUIRE(list_bool_type);
            REQUIRE(list_bool_type.is_equal_to(list_bool_type));

            auto any_type = TypeOps::atomic_type(alloc, Metatype::t_any());
            auto list_any_type = TypeOps::list_type(alloc, any_type);

            REQUIRE(list_any_type);
            REQUIRE(list_any_type.is_equal_to(list_any_type));

            REQUIRE(list_bool_type.is_subtype_of(list_any_type));
            REQUIRE(list_i64_type.is_subtype_of(list_any_type));

            REQUIRE(!list_i64_type.is_subtype_of(list_bool_type));
            REQUIRE(!list_any_type.is_subtype_of(list_bool_type));
            REQUIRE(!list_bool_type.is_subtype_of(list_i64_type));
            REQUIRE(!list_any_type.is_subtype_of(list_i64_type));
        }
    }
}


/* end DListType.cpp */
