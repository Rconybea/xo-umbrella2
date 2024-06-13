/* @file Variable.cpp */

#include "Variable.hpp"

namespace xo {
    namespace ast {
        void
        Variable::display(std::ostream & os) const {
            os << "<Variable"
               << xtag("name", name_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end Variable.cpp */
