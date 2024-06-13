/* @file Apply.cpp */

#include "Apply.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace ast {
        void
        Apply::display(std::ostream & os) const {
            os << "<Apply"
               << xtag("fn", fn_)
               << xtag("argv", argv_)
               << ">";
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end Apply.cpp */
