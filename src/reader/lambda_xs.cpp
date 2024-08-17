/* @file lambda_xs.cpp */

#include "lambda_xs.hpp"
#include "expect_formal_arglist_xs.hpp"
#include "expect_expr_xs.hpp"
#include "xo/expression/Lambda.hpp"

namespace xo {
    using xo::ast::Lambda;

    namespace scm {
        std::unique_ptr<lambda_xs>
        lambda_xs::make() {
            return std::make_unique<lambda_xs>(lambda_xs());
        }

        lambda_xs::lambda_xs() {}

        void
        lambda_xs::on_lambda_token(const token_type & tk,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr)
        {
            if (lmxs_type_ == lambdastatetype::lm_0) {
                this->lmxs_type_ = lambdastatetype::lm_1;
                p_stack->push_exprstate(expect_formal_arglist_xs::make());
                return;
            }

            exprstate::on_lambda_token(tk,
                                       p_stack,
                                       p_emit_expr);
        }

        void
        lambda_xs::on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                     exprstatestack * p_stack,
                                     rp<Expression> * p_emit_expr)
        {
            if (lmxs_type_ == lambdastatetype::lm_1) {
                this->lmxs_type_ = lambdastatetype::lm_2;
                this->argl_ = argl;
                p_stack->push_exprstate(expect_expr_xs::expect_rhs_expression());
                return;
            }

            exprstate::on_formal_arglist(argl,
                                         p_stack,
                                         p_emit_expr);
        }

        void
        lambda_xs::on_expr(ref::brw<Expression> expr,
                           exprstatestack * p_stack,
                           rp<Expression> * p_emit_expr)
        {
            if (lmxs_type_ == lambdastatetype::lm_2) {
                this->lmxs_type_ = lambdastatetype::lm_3;
                this->body_ = expr.promote();
                return;
            }

            exprstate::on_expr(expr, p_stack, p_emit_expr);
        }

        void
        lambda_xs::on_semicolon_token(const token_type & tk,
                                      exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr)
        {
            if (lmxs_type_ == lambdastatetype::lm_3) {
                /* done! */

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                std::string name = "fixmename";

                rp<Lambda> lm = Lambda::make(name, argl_, body_);

                p_stack->top_exprstate().on_expr(lm, p_stack, p_emit_expr);
                p_stack->top_exprstate().on_semicolon_token(tk, p_stack, p_emit_expr);

                return;
            }

            exprstate::on_semicolon_token(tk, p_stack, p_emit_expr);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end lambda_xs.cpp */
