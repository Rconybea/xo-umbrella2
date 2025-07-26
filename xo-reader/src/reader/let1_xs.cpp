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
    using Environment = xo::ast::Environment;
    using LocalEnv = xo::ast::LocalEnv;
    using Variable = xo::ast::Variable;

    namespace scm {
        std::unique_ptr<let1_xs>
        let1_xs::make(std::string lhs_name,
                      rp<LocalEnv> local_env,
                      rp<Expression> rhs)
        {
            return std::make_unique<let1_xs>(let1_xs(std::move(lhs_name),
                                                     std::move(local_env),
                                                     std::move(rhs)));
        }

        void
        let1_xs::start(const std::string & lhs_name,
                       const rp<Expression> & rhs,
                       parserstatemachine * p_psm)
        {
            rp<Environment> parent_env = p_psm->top_envframe().promote();
            rp<Variable> var1 = Variable::make(lhs_name, rhs->valuetype());
            rp<LocalEnv> let_env = LocalEnv::make1(var1, parent_env);

            p_psm->push_envframe(let_env);

            // TODO: stash let_env in let1_xs, then pick up directly in .on_rightbrace_token()
            //       still have to push here so vars can find it
            //
            p_psm->push_exprstate(let1_xs::make(lhs_name, let_env, rhs));

            expect_expr_xs::start(true /*allow_defs*/,
                                  true /*cxl_on_rightbrace*/,
                                  p_psm);
        }

        let1_xs::let1_xs(std::string lhs_name,
                         rp<LocalEnv> local_env,
                         rp<Expression> rhs)
            : exprstate(exprstatetype::let1expr),
              lhs_name_{std::move(lhs_name)},
              local_env_{std::move(local_env)},
              rhs_{std::move(rhs)}
        {}

        void
        let1_xs::on_expr(bp<Expression> expr,
                         parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            bp<DefineExpr> def_expr = DefineExpr::from(expr);

            if (def_expr) {
                /** starting a nested let here:
                 *  control returns via
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
        let1_xs::on_expr_with_semicolon(bp<Expression> expr,
                                        parserstatemachine * p_psm)
        {
            /* same as on_expr(), since we only use let1_xs inside a block { .. }
             * This means final ';' is unnecessary
             */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            bp<DefineExpr> def_expr = DefineExpr::from(expr);

            if (def_expr) {
                let1_xs::start(def_expr->lhs_name(),
                               def_expr->rhs(),
                               p_psm);
            } else {
                this->expr_v_.push_back(expr.promote());

                expect_expr_xs::start(true /*allow_defs*/,
                                      true /*cxl_on_rightbvrace*/,
                                      p_psm);
            }
        }

        void
        let1_xs::on_rightbrace_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            auto self = p_psm->pop_exprstate();

            auto expr = Sequence::make(this->expr_v_);

            /* top env frame was established by let1_xs::start();
             * now unwind it
             */
            p_psm->pop_envframe();

            std::string lambda_name = Variable::gensym("let1");

            rp<Environment> parent_env = p_psm->top_envframe().promote();

            rp<Expression> lambda
                = Lambda::make_from_env(lambda_name,
                                        local_env_,
                                        nullptr /*explicit_return_td*/,
                                        expr);

            rp<Expression> result
                = Apply::make(lambda, {this->rhs_});

            p_psm->top_exprstate().on_expr(result, p_psm);

            /* caller of let1_xs expects the same rightbrace '}'
             * -- remember we pushed let1_xs to handle an embedded def-expr
             * in a sequence
             */
            p_psm->on_rightbrace_token(tk);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end let1_xs.cpp */
