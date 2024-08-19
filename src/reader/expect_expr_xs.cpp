/* file expect_expr_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "lambda_xs.hpp"
#include "paren_xs.hpp"
#include "progress_xs.hpp"
#include "xo/expression/Lambda.hpp"
#include "xo/expression/Constant.hpp"

namespace xo {
    using xo::ast::Constant;

    namespace scm {

        std::unique_ptr<expect_expr_xs>
        expect_expr_xs::make() {
            return std::make_unique<expect_expr_xs>(expect_expr_xs());

        }

        void
        expect_expr_xs::start(exprstatestack * p_stack) {
            p_stack->push_exprstate(expect_expr_xs::make());
        }

        expect_expr_xs::expect_expr_xs()
            : exprstate(exprstatetype::expect_rhs_expression)
        {}

        void
        expect_expr_xs::on_lambda_token(const token_type & /*tk*/,
                                        parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            auto p_stack = p_psm->p_stack_;
            auto p_emit_expr = p_psm->p_emit_expr_;

            /* push lparen_0 to remember to look for subsequent rightparen. */
            lambda_xs::start(p_stack, p_emit_expr);
            //p_stack->top_exprstate().on_lambda_token(tk, p_stack, p_emit_expr);
            //p_stack->push_exprstate(expect_expr_xs::expect_rhs_expression());
        }

        void
        expect_expr_xs::on_leftparen_token(const token_type & /*tk*/,
                                           exprstatestack * p_stack,
                                           rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            /* push lparen_0 to remember to look for subsequent rightparen. */
            paren_xs::start(p_stack);
        }

        void
        expect_expr_xs::on_symbol_token(const token_type & /*tk*/,
                                        exprstatestack * /*p_stack*/,
                                        rp<Expression> * /*p_emit_expr*/)
        {
            /* todo: treat symbol as variable name */

            /* various possibilities when looking for rhs expression:
             *
             *   x := y       // (1)
             *   x := f(a)    // (2)
             *   x := f(a,b)  // (3)
             *
             * need lookahead token following symbol to distinguish
             * between (1) (symbol completes rhs expression)
             * and {(2), (3)} (symbol is function call)
             */

            /* have to do pop first, before sending symbol to
             * the o.g. symbol-requester
             */
#ifdef NOT_YET
            p_stack->push_exprstate(exprstate(exprstatetype::expr_progress,
                                              Variable::make(name, type)));
#endif

#ifdef LATER
            p_stack->pop_exprstate();
            p_stack->top_exprstate().on_symbol(tk.text(),
                                               p_stack, p_emit_expr);
#endif
            return;
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
            progress_xs::start
                (Constant<double>::make(tk.f64_value()),
                 p_stack);
        }

        void
        expect_expr_xs::on_expr(ref::brw<Expression> expr,
                                exprstatestack * p_stack,
                                rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr));


            std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

            p_stack->top_exprstate().on_expr(expr,
                                             p_stack,
                                             p_emit_expr);
        } /*on_expr*/

    } /*namespace scm*/
} /*namespace xo*/

/* end expect_expr_xs.cpp */
