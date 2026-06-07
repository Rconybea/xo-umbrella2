/** @file IAllocator_Any.test.cpp
 *
 *  @author Roland Conybeare, May 2026
 **/

#include <xo/testutil/Utest.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <catch2/catch.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

namespace xo {
    using xo::mm::AAllocator;

    namespace ut {

        TEST_CASE("IAllocator_Any", "[alloc2][death]")
        {
            auto log = Utest::ut_scope();

            // null allocator
            obj<AAllocator> alloc_any;

            // NOTE: tried using a fork() strategy to verify termination,
            //       but child process doesn't get measured by gcov
        }

    } /*namespace ut*/
} /*namespace xo*/

/* end IAllocator_Any.test.cpp */
