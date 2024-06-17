/* @file IfExpr.cpp */

#include "IfExpr.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace ast {
        void
        IfExpr::display(std::ostream & os) const {
            os << "<IfExpr"
               << xtag("test", test_)
               << xtag("when_true", when_true_)
               << xtag("when_false", when_false_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end IfExpr.cpp */
