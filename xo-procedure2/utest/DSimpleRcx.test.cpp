/** @file DSimpleRcx.test.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include <xo/procedure2/init_procedure2.hpp>
#include <xo/procedure2/DSimpleRcx.hpp>
#include <xo/procedure2/detail/IRuntimeContext_DSimpleRcx.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <catch2/catch.hpp>

namespace xo {
    using xo::scm::DSimpleRcx;
    using xo::scm::ARuntimeContext;
    using xo::mm::AAllocator;
    using xo::mm::DArena;
    using xo::mm::ArenaConfig;
    using xo::facet::with_facet;
    using xo::facet::obj;

    namespace ut {
        static InitEvidence s_init = InitSubsys<S_procedure2_tag>::require();

        TEST_CASE("DSimpleRcx-init", "[procedure2][DSimpleRcx]")
        {
            REQUIRE(s_init.evidence());
        }

        TEST_CASE("DSimpleRcx-construct", "[procedure2][DSimpleRcx]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DSimpleRcx rcx(alloc);

            REQUIRE((void*)rcx.allocator().data() == (void*)alloc.data());
        }

        TEST_CASE("DSimpleRcx-as-ARuntimeContext", "[procedure2][DSimpleRcx]")
        {
            ArenaConfig cfg { .name_ = "testarena",
                              .size_ = 4*1024 };
            DArena arena = DArena::map(cfg);
            auto alloc = with_facet<AAllocator>::mkobj(&arena);

            DSimpleRcx rcx(alloc);
            obj<ARuntimeContext> rcx_obj = with_facet<ARuntimeContext>::mkobj(&rcx);

            // verify we can recover allocator from obj<ARuntimeContext>
            obj<AAllocator> recovered_alloc = rcx_obj.allocator();

            REQUIRE((void*)recovered_alloc.data() == (void*)alloc.data());
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end DSimpleRcx.test.cpp */
