/** @file appcx_indentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "appcx_indentlog2.hpp"

namespace xo {

    Indentlog2_Appcx::Indentlog2_Appcx(const Indentlog2_Config & cfg)
        : config_{cfg}
    {
        TempArena::init(config_.temp_arena_capacity());
    }

} /*namespace xo*/

/* end appcx_indentlog2.cpp */
