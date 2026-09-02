/** @file config_indentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "config_indentlog2.hpp"

namespace xo {
    Indentlog2_Config::Indentlog2_Config(const PpConfig & pp_cfg, uint32_t cap)
        : init_evidence_{InitSubsys<S_indentlog2_tag>::require()},
          pp_config_{pp_cfg},
          temp_arena_capacity_{cap}
    {}
}

/* end config_indentlog2.cpp */
