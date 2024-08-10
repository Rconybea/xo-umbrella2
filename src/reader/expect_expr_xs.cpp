/* file expect_expr_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_expr_xs.hpp"

namespace xo {
    namespace scm {

        std::unique_ptr<expect_expr_xs>
        expect_expr_xs::expect_rhs_expression() {
            return std::make_unique<expect_expr_xs>(expect_expr_xs());

        }

        expect_expr_xs::expect_expr_xs()
            : exprstate(exprstatetype::expect_rhs_expression)
        {}

    } /*namespace scm*/
} /*namespace xo*/

/* end expect_expr_xs.cpp */
