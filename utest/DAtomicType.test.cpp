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

            auto f64_type = obj<AType,DAtomicType>(DAtomicType::_make(alloc, Metatype::unit()));

            REQUIRE(f64_type);
            REQUIRE(f64_type.metatype().code() == Metatype::unit().code());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DAtomicType.test.cpp */
