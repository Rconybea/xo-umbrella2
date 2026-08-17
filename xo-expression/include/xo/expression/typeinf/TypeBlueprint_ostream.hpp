/** @file TypeBlueprint_ostream.hpp **/

#pragma once

#include "TypeBlueprint.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <ostream>

namespace xo::scm {
    inline std::ostream &
    operator<<(std::ostream & os, const TypeBlueprint & x) {
        using xo::pp::tostr;

        os << tostr(x);
        return os;
    }
}

/* end TypeBlueprint_ostream.hpp */
