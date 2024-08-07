/* @file exprstate.cpp */

#include "exprstate.hpp"
#include "define_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<define_xs>
        define_xs::def_0(rp<DefineExprAccess> def_expr) {
            return std::make_unique<define_xs>(define_xs(def_expr));
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end exprstate.cpp */
