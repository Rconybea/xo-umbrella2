/** @file config_indentlog2.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "config_indentlog2.hpp"

namespace xo {
    Indentlog2_Config::Indentlog2_Config(uint32_t cap)
        : init_evidence_{InitSubsys<S_indentlog2_tag>::require()},
          temp_arena_capacity_{cap}
    {}
}

/* end config_indentlog2.cpp */
