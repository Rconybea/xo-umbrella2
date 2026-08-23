/** @file type_unifier_ostream.hpp **/

#pragma once

#include "type_unifier.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <ostream>

namespace xo::scm {
    inline std::ostream &
    operator<< (std::ostream & os, const unify_result & x) {
        using xo::pp::tostr;

        os << tostr(x);
        return os;
    }
}

/* end type_unifier_ostream.hpp */
