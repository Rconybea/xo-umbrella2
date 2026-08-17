/** @file type_unifier_ostream.hpp **/

#pragma once

#include "type_unifier.hpp"
#include <ostream>

namespace xo::scm {
    inline std::ostream &
    operator<< (std::ostream & os, const unify_result & x) {
        x.print(os);
        return os;
    }
}

/* end type_unifier_ostream.hpp */
