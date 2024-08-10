/* file expect_expr_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_expr_xs.hpp"
#include "paren_xs.hpp"
#include "progress_xs.hpp"
#include "xo/expression/Constant.hpp"

namespace xo {
    using xo::ast::Constant;

    namespace scm {

        std::unique_ptr<expect_expr_xs>
        expect_expr_xs::expect_rhs_expression() {
            return std::make_unique<expect_expr_xs>(expect_expr_xs());

        }

        expect_expr_xs::expect_expr_xs()
            : exprstate(exprstatetype::expect_rhs_expression)
        {}

        void
        expect_expr_xs::on_leftparen_token(const token_type & /*tk*/,
                                           exprstatestack * p_stack,
                                           rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            /* push lparen_0 to remember to look for subsequent rightparen. */
            p_stack->push_exprstate(paren_xs::lparen_0());
            p_stack->push_exprstate(expect_expr_xs::expect_rhs_expression());
        }

        void
        expect_expr_xs::on_f64_token(const token_type & tk,
                                     exprstatestack * p_stack,
                                     rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_f64_token";

            /* e.g.
             *   def pi = 3.14159265;
             *            \---tk---/
             */
            p_stack->push_exprstate
                (progress_xs::make
                 (Constant<double>::make(tk.f64_value())));
        }


    } /*namespace scm*/
} /*namespace xo*/

/* end expect_expr_xs.cpp */
