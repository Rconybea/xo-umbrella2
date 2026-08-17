/** @file SymbolTable.cpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#include "SymbolTable.hpp"
#include <xo/indentlog2/print/tostr.hpp>

namespace xo::scm {
    std::string
    SymbolTable::display_string() const {
        using xo::pp::tostr;

        return tostr(*this);
    }
} /*namespace xo::scm*/

/* end SymbolTable.cpp */
