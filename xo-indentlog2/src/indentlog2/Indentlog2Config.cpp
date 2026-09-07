/** @file Indentlog2Config.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/Indentlog2Config.hpp"

namespace xo {

    Indentlog2Config::Indentlog2Config(const PpConfig & pp_cfg, uint32_t cap)
        : pp_config_{pp_cfg},
          temp_arena_capacity_{cap}
    {}

    Indentlog2Config
    Indentlog2Config::make_default()
    {
        return Indentlog2Config(PpConfig::colored(),
                                64 * 1024);
    }

} /*namespace xo*/

/* end Indentlog2Config.cpp */
