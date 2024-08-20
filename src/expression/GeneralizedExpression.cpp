/* @file GeneralizedExpression.cpp */

#include "GeneralizedExpression.hpp"

namespace xo {
    namespace ast {
        std::string
        GeneralizedExpression::display_string() const {
            return tostr(*this);
        }
    } /*namespace ast*/
} /*namespace xo*/

/* end GeneralizedExpression.cpp */
