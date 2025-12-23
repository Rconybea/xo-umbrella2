/** @file random_allocs.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include "Allocator.hpp"
#include <xo/facet/obj.hpp>
#include <xo/randomgen/xoshiro256.hpp>

namespace utest {

/* note: trivial REQUIRE() call in else branch bc we still want
 *       catch2 to count assertions when verification succeeds
 */
#  define REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr) \
    if (catch_flag) {                                  \
        REQUIRE((expr));                               \
    } else {                                           \
        REQUIRE(true);                                 \
        ok_flag &= (expr);                             \
    }

#  define REQUIRE_ORFAIL(ok_flag, catch_flag, expr)    \
    REQUIRE_ORCAPTURE(ok_flag, catch_flag, expr);      \
    if (!ok_flag)                                      \
        return ok_flag



    struct AllocUtil {
        using AAllocator = xo::mm::AAllocator;

        /** generate a random sequence of allocations.
         *  verify allocator behavior
         **/
        static bool random_allocs(std::uint32_t n_alloc,
                                  bool catch_flag,
                                  xo::rng::xoshiro256ss * p_rgen,
                                  xo::facet::obj<AAllocator> alloc);
    };
}

/* end random_allocs.hpp */
