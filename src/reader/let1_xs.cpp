/* file let1_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "let1_xs.hpp"
#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "xo/expression/Sequence.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Apply.hpp"
#include "xo/expression/Lambda.hpp"

namespace xo {
    using Sequence = xo::ast::Sequence;
    using DefineExpr = xo::ast::DefineExpr;
    using Apply = xo::ast::Apply;
    using Lambda = xo::ast::Lambda;
    using LambdaAccess = xo::ast::LambdaAccess;
    using Variable = xo::ast::Variable;

    namespace {
        std::string gensym() {
            return "genanotherxx";
        }
    }

    namespace scm {
        std::unique_ptr<let1_xs>
        let1_xs::make(std::string lhs_name,
                      rp<Expression> rhs)
        {
            return std::make_unique<let1_xs>(let1_xs(std::move(lhs_name),
                                                     std::move(rhs)));
        }

        void
        let1_xs::start(const std::string & lhs_name,
                       const rp<Expression> & rhs,
                       parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(let1_xs::make(std::move(lhs_name),
                                                std::move(rhs)));

            expect_expr_xs::start(true /*allow_defs*/,
                                  true /*cxl_on_rightbrace*/,
                                  p_psm);
        }

        let1_xs::let1_xs(std::string lhs_name,
                         rp<Expression> rhs)
            : exprstate(),
              lhs_name_{std::move(lhs_name)},
              rhs_{std::move(rhs)}
        {}

        void
        let1_xs::on_expr(ref::brw<Expression> expr,
                         parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            ref::brw<DefineExpr> def_expr = DefineExpr::from(expr);

            if (def_expr) {
                /** nested_start: control returns via
                 *   .on_expr(x)
                 *      with x something like:
                 *    Apply(Lambda(gensym(),
                 *                 [Variable(def_expr->lhs_name(),
                 *                           def_expr->valuetype())],
                 *                 body...))
                 *  followed immediately by
                 *   .on_rightbrace_token()
                 **/
                let1_xs::start(def_expr->lhs_name(),
                               def_expr->rhs(),
                               p_psm);
            } else {
                this->expr_v_.push_back(expr.promote());

                expect_expr_xs::start(true /*allow_defs*/,
                                      true /*cxl_on_rightbrace*/,
                                      p_psm);
            }
        }

        void
        let1_xs::on_rightbrace_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            auto self = p_psm->pop_exprstate();

            auto expr = Sequence::make(this->expr_v_);

            std::string argname = gensym();

            rp<Expression> lambda
                = Lambda::make(this->lhs_name_,
                               {Variable::make(argname,
                                               this->rhs_->valuetype())},
                               expr);

            rp<Expression> result
                = Apply::make(lambda, {this->rhs_});

            p_psm->top_exprstate().on_expr(result, p_psm);

            /* caller of let1_xs expects the same rightbrace '}'
             * -- remember we pushed let1_xs to handle an embedded def-expr
             * in a sequence
             */
            p_psm->top_exprstate().on_rightbrace_token(tk, p_psm);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end let1_xs.cpp */
