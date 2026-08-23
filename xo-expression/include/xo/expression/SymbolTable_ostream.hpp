/** @file SymbolTable_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include "SymbolTable.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <iostream>

namespace xo::scm {
    inline std::ostream &
    operator<< (std::ostream & os,
                const SymbolTable & x) {
        using xo::pp::tostr;

        os << tostr(x);
        return os;
    }
}

/* end SymbolTable_ostream.hpp */
