/* @file lambda_xs.cpp */

#include "lambda_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
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

        void
        lambda_xs::start(parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(lambda_xs::make());
            p_psm->top_exprstate()
                .on_lambda_token(token_type::lambda(), p_psm);
        }

        lambda_xs::lambda_xs() : exprstate(exprstatetype::lambdaexpr) {}

        void
        lambda_xs::on_lambda_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            auto p_stack = p_psm->p_stack_;

            if (lmxs_type_ == lambdastatetype::lm_0) {
                this->lmxs_type_ = lambdastatetype::lm_1;
                expect_formal_arglist_xs::start(p_stack);
            } else {
                exprstate::on_lambda_token(tk, p_psm);
            }
        }

        void
        lambda_xs::on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                     parserstatemachine * p_psm)
        {
            auto p_stack = p_psm->p_stack_;

            if (lmxs_type_ == lambdastatetype::lm_1) {
                this->lmxs_type_ = lambdastatetype::lm_2;
                this->argl_ = argl;
                expect_expr_xs::start(p_stack);
            } else {
                exprstate::on_formal_arglist(argl, p_psm);
            }
        }

        void
        lambda_xs::on_expr(ref::brw<Expression> expr,
                           parserstatemachine * p_psm)
        {
            if (lmxs_type_ == lambdastatetype::lm_2) {
                this->lmxs_type_ = lambdastatetype::lm_3;
                this->body_ = expr.promote();
            } else {
                exprstate::on_expr(expr, p_psm);
            }
        }

        void
        lambda_xs::on_semicolon_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            if (lmxs_type_ == lambdastatetype::lm_3) {
                /* done! */

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                std::string name = "fixmename";

                rp<Lambda> lm = Lambda::make(name, argl_, body_);

                p_psm->top_exprstate().on_expr(lm, p_psm);
                p_psm->top_exprstate().on_semicolon_token(tk, p_psm);

                return;
            }

            exprstate::on_semicolon_token(tk, p_psm);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end lambda_xs.cpp */
