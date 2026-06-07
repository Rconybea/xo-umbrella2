/** @file DAtomicType.test.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "init_type.hpp"
#include "AtomicType.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/alloc2/Arena.hpp>
#include <xo/facet/obj.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::AType;
    using xo::scm::DAtomicType;
    using xo::scm::Metatype;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;

    namespace ut {
        static InitEvidence s_init = (InitSubsys<S_type_tag>::require());

        TEST_CASE("DAtomicType-make", "[type][DAtomicType]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = obj<AAllocator,DArena>(&arena);

            auto unit_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::t_unit()));
            auto i64_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::t_i64()));
            auto f64_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::t_f64()));
            auto str_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::t_str()));
            auto any_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::t_any()));
            {
                REQUIRE(unit_type);
                REQUIRE(unit_type.metatype().code() == Metatype::t_unit().code());

                REQUIRE(unit_type.is_equal_to(unit_type));
                REQUIRE(unit_type.is_subtype_of(unit_type));
            }

            {
                REQUIRE(i64_type);
                REQUIRE(i64_type.metatype().code() == Metatype::t_i64().code());

                REQUIRE(i64_type.is_equal_to(i64_type));
                REQUIRE(i64_type.is_subtype_of(i64_type));

                REQUIRE(!i64_type.is_subtype_of(unit_type));
                REQUIRE(!unit_type.is_subtype_of(i64_type));
            }

            {
                REQUIRE(f64_type);
                REQUIRE(f64_type.metatype().code() == Metatype::t_f64().code());

                REQUIRE(f64_type.is_equal_to(f64_type));
                REQUIRE(f64_type.is_subtype_of(f64_type));

                REQUIRE(!f64_type.is_subtype_of(unit_type));
                REQUIRE(!f64_type.is_subtype_of(i64_type));
                REQUIRE(!unit_type.is_subtype_of(f64_type));
                REQUIRE(!i64_type.is_subtype_of(f64_type));
            }

            {
                REQUIRE(str_type);
                REQUIRE(str_type.metatype().code() == Metatype::t_str().code());

                REQUIRE(str_type.is_equal_to(str_type));
                REQUIRE(str_type.is_subtype_of(str_type));

                REQUIRE(!str_type.is_subtype_of(unit_type));
                REQUIRE(!str_type.is_subtype_of(i64_type));
                REQUIRE(!str_type.is_subtype_of(f64_type));

                REQUIRE(!unit_type.is_subtype_of(str_type));
                REQUIRE(!i64_type.is_subtype_of(str_type));
                REQUIRE(!f64_type.is_subtype_of(str_type));
            }

            {
                REQUIRE(any_type);
                REQUIRE(any_type.metatype().code() == Metatype::t_any().code());

                REQUIRE(any_type.is_equal_to(any_type));
                REQUIRE(any_type.is_subtype_of(any_type));

                REQUIRE(!any_type.is_subtype_of(unit_type));
                REQUIRE(!any_type.is_subtype_of(i64_type));
                REQUIRE(!any_type.is_subtype_of(f64_type));
                REQUIRE(!any_type.is_subtype_of(str_type));

                REQUIRE(unit_type.is_subtype_of(any_type));
                REQUIRE(i64_type.is_subtype_of(any_type));
                REQUIRE(f64_type.is_subtype_of(any_type));
                REQUIRE(str_type.is_subtype_of(any_type));
            }
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DAtomicType.test.cpp */
