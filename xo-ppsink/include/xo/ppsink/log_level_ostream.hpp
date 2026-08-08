/** @file log_level_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, log_level): print a log_level by name.
 *
 *  Segregated from log_level.hpp, which stays free of <ostream> (same split as
 *  tag.hpp vs tag_ostream.hpp).  The legacy xo-indentlog log_level.hpp declared
 *  its inserter inline, so every includer paid for <ostream>.
 *
 *  Include this where a log_level is streamed -- including where it is a legacy
 *  xo::xtag value, since that renders through operator<<.
 **/

#pragma once

#include "log_level.hpp"
#include <ostream>

namespace xo::pp {
    inline std::ostream &
    operator<<(std::ostream & os, log_level x) {
        os << log_level_name(x);
        return os;
    }
} /*namespace xo::pp*/

/* end log_level_ostream.hpp */
