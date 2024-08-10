/* file expect_symbol_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_symbol_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<expect_symbol_xs>
        expect_symbol_xs::expect_symbol_expression() {
            return std::make_unique<expect_symbol_xs>(expect_symbol_xs());
        }

        expect_symbol_xs::expect_symbol_xs()
            : exprstate(exprstatetype::expect_symbol)
        {}

        void
        expect_symbol_xs::on_symbol_token(const token_type & tk,
                                          exprstatestack * p_stack,
                                          rp<Expression> * p_emit_expr)
        {
            /* have to do pop first, before sending symbol to
             * the o.g. symbol-requester
             */
            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            p_stack->top_exprstate().on_symbol(tk.text(),
                                               p_stack, p_emit_expr);
            return;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end expect_symbol_xs.cpp */
