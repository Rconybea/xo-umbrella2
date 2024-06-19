/* @file Variable.cpp */

#include "Variable.hpp"

namespace xo {
    namespace ast {
        void
        Variable::display(std::ostream & os) const {
            os << "<Variable"
               << xtag("name", name_)
               << xtag("type", this->valuetype()->short_name())
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end Variable.cpp */
