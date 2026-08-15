/** @file SetupIndentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "SetupIndentlog2.hpp"
#include "TempArena.hpp"

namespace xo {
    bool
    SetupIndentlog2::configure_temp_arena(std::uint32_t cap)
    {
        mm::TempArena::init(cap);

        return true;
    }

} /*namespace xo*/

/* end SetupIndentlog2.cpp */
