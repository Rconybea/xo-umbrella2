/* @file Expression.cpp */

#include "Expression.hpp"

namespace xo {
    namespace ast {
        std::string
        Expression::display_string() const {
            return tostr(*this);
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end Expression.cpp */
