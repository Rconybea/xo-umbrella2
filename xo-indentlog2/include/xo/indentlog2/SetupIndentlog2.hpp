/** @file SetupIndentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    class SetupIndentlog2 {
    public:
        using uint32_t = std::uint32_t;

    public:
        /** Setup per-thread scratch arenas; reserve cap **/
        static bool configure_temp_arena(uint32_t cap);
    };
}

/* end SetupIndentlog2.hpp */
