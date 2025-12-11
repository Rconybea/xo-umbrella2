/** @file arena.test.cpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#include "xo/alloc2/AAllocator.hpp"
#include "xo/alloc2/DArena.hpp"
#include "xo/alloc2/IAllocator_DArena.hpp"
#include "xo/alloc2/padding.hpp"
#include <catch2/catch.hpp>

namespace xo {
    using xo::mm::IAllocator_Xfer;
    using xo::mm::DArena;

    namespace ut {
        TEST_CASE("IAllocator_Xfer_DArena", "[alloc2]")
        {
            IAllocator_Xfer<DArena> xfer;

            REQUIRE(IAllocator_Xfer<DArena>::_valid);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end arena.test.cpp */
