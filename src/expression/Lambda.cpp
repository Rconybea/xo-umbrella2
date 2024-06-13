/* @file Lambda.cpp */

#include "Lambda.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace ast {
        void
        Lambda::display(std::ostream & os) const {
            os << "<Lambda"
               << xtag("name", name_)
               << xtag("argv", argv_)
               << xtag("body", body_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end Lambda.cpp */
