/** @file DArrayType.test.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "init_type.hpp"
#include "TypeOps.hpp"
#include "ArrayType.hpp"
#include "ListType.hpp"
#include "AtomicType.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/obj.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::TypeOps;
    using xo::scm::AType;
    using xo::scm::Metatype;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_type_tag>::require());

        TEST_CASE("DArrayType-make", "[type][DArrayType]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = obj<AAllocator,DArena>(&arena);

            auto i64_type = TypeOps::atomic_type(alloc, Metatype::t_i64());
            auto array_i64_type = TypeOps::array_type(alloc, i64_type);

            REQUIRE(array_i64_type);
            REQUIRE(array_i64_type.is_equal_to(array_i64_type));

            auto bool_type = TypeOps::atomic_type(alloc, Metatype::t_bool());
            auto array_bool_type = TypeOps::array_type(alloc, bool_type);

            REQUIRE(array_bool_type);
            REQUIRE(array_bool_type.is_equal_to(array_bool_type));

            auto any_type = TypeOps::atomic_type(alloc, Metatype::t_any());
            auto array_any_type = TypeOps::array_type(alloc, any_type);

            REQUIRE(array_any_type);
            REQUIRE(array_any_type.is_equal_to(array_any_type));

            REQUIRE(array_bool_type.is_subtype_of(array_any_type));
            REQUIRE(array_i64_type.is_subtype_of(array_any_type));

            REQUIRE(!array_i64_type.is_subtype_of(array_bool_type));
            REQUIRE(!array_any_type.is_subtype_of(array_bool_type));
            REQUIRE(!array_bool_type.is_subtype_of(array_i64_type));
            REQUIRE(!array_any_type.is_subtype_of(array_i64_type));

            // array and list are unrelated
            auto list_i64_type = TypeOps::list_type(alloc, i64_type);
            REQUIRE(!array_i64_type.is_equal_to(list_i64_type));
            REQUIRE(!array_i64_type.is_subtype_of(list_i64_type));
            REQUIRE(!list_i64_type.is_subtype_of(array_i64_type));
        }
    }
}


/* end DArrayType.test.cpp */
