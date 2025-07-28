/* @file lambda_xs.cpp */

#include "lambda_xs.hpp"
#include "define_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "expect_formal_arglist_xs.hpp"
#include "expect_expr_xs.hpp"
#include "expect_type_xs.hpp"
#include "xo/expression/Lambda.hpp"

namespace xo {
    using xo::scm::Lambda;
    using xo::scm::LocalEnv;

    namespace scm {
        const char *
        lambdastatetype_descr(lambdastatetype x) {
            switch(x) {
            case lambdastatetype::invalid: return "invalid";
            case lambdastatetype::lm_0: return "lm_0";
            case lambdastatetype::lm_1: return "lm_1";
            case lambdastatetype::lm_2: return "lm_2";
            case lambdastatetype::lm_3: return "lm_3";
            case lambdastatetype::lm_4: return "lm_4";
            case lambdastatetype::lm_5: return "lm_5";
            default: break;
            }

            return "???lambdastatetype";
        }

        // ----- lambda_xs - ----

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

        const char *
        lambda_xs::get_expect_str() const
        {
            /*
             *   lambda (x : f64) : f64 { ... } ;
             *  ^      ^         ^ ^   ^       ^
             *  |      |         | |   |       lm_5
             *  |      |         | |   lm_4:expect_expression
             *  |      |         | lm_3
             *  |      |         lm_2
             *  |      lm_1:
             *  expect_expression
             */
             switch (this->lmxs_type_) {
             case lambdastatetype::invalid:
             case lambdastatetype::lm_0:
             case lambdastatetype::n_lambdastatetype:
                 assert(false); // impossible
                 return nullptr;
             case lambdastatetype::lm_1:
                 return "lambda-params";
             case lambdastatetype::lm_2:
                 return "colon|lambda-body";
             case lambdastatetype::lm_3:
                 return "type";
             case lambdastatetype::lm_4:
                 return "lambda-body";
             case lambdastatetype::lm_5:
                 return "semicolon";
             }

             return "?expect";
        }

        void
        lambda_xs::on_lambda_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            if (lmxs_type_ == lambdastatetype::lm_0) {
                this->lmxs_type_ = lambdastatetype::lm_1;
                expect_formal_arglist_xs::start(p_psm);
            } else {
                exprstate::on_lambda_token(tk, p_psm);
            }
        }

        void
        lambda_xs::on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                     parserstatemachine * p_psm)
        {
            if (lmxs_type_ == lambdastatetype::lm_1) {
                this->lmxs_type_ = lambdastatetype::lm_2;
                this->parent_env_ = p_psm->top_envframe().promote();
                this->local_env_ = LocalEnv::make(argl, parent_env_);

                p_psm->push_envframe(local_env_);

                //expect_expr_xs::start(p_psm);
            } else {
                exprstate::on_formal_arglist(argl, p_psm);
            }
        }

        void
        lambda_xs::on_expr_with_semicolon(bp<Expression> expr,
                                          parserstatemachine * p_psm)
        {
            this->on_expr(expr, p_psm);
            this->on_semicolon_token(token_type::semicolon(), p_psm);
        }

        void
        lambda_xs::on_colon_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "lambda_xs::on_colon_token";

            if (lmxs_type_ == lambdastatetype::lm_2) {
                this->lmxs_type_ = lambdastatetype::lm_3;
                expect_type_xs::start(p_psm);
                /* control reenters via .on_typedescr() */
            } else {
                this->illegal_input_on_token(c_self_name, tk, this->get_expect_str(), p_psm);
            }
        }

        void
        lambda_xs::on_leftbrace_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "lambda_xs::on_leftbrace_token";

            if (lmxs_type_ == lambdastatetype::lm_2)
                this->lmxs_type_ = lambdastatetype::lm_4;

            if (lmxs_type_ == lambdastatetype::lm_4) {
                expect_expr_xs::start(p_psm);
                /* want { to start expr sequence, that finishes on matching } */
                p_psm->on_leftbrace_token(token_type::leftbrace());
            } else {
                this->illegal_input_on_token(c_self_name, tk, this->get_expect_str(), p_psm);
            }
        }

        void
        lambda_xs::on_typedescr(TypeDescr td,
                                parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "lambda_xs::on_typedescr";
            scope log(XO_DEBUG(p_psm->debug_flag()));

            assert(td);

            if (lmxs_type_ == lambdastatetype::lm_3) {
                this->lmxs_type_ = lambdastatetype::lm_4;
                this->explicit_return_td_ = td;

                this->lambda_td_ = Lambda::assemble_lambda_td(local_env_->argv(),
                                                              explicit_return_td_);

                /* 1. at this point we know function signature (@ref lambda_td_)
                 * 2. if this lambda appears on the rhs of a define,
                 *    propagate function signature to the define.
                 * 3. this makes recursive function definitions like this work
                 *    without relying on type inference:
                 *       def fact = lambda (n : i64) : i64 {
                 *         if (n == 0) then
                 *           1
                 *         else
                 *           n * fact(n - 1)
                 *       }
                 * 4. while parsing the body of the lambda, we want environment
                 *    to already associate the lambda's signature with variable 'fact',
                 *    so that when parser encounters 'fact(n - 1)' the expression has
                 *    known valuetype.
                 */

                if ((p_psm->exprstate_stack_size() >= 3)
                    && (p_psm->lookup_exprstate(1).exs_type() == exprstatetype::expect_rhs_expression)
                    && (p_psm->lookup_exprstate(2).exs_type() == exprstatetype::defexpr)
                    && (p_psm->env_stack_size() >= 2)
                    )
                {
                    const define_xs * def_xs = dynamic_cast<const define_xs*>(&(p_psm->lookup_exprstate(2)));

                    assert(def_xs);

                    bp<Variable> def_var = def_xs->lhs_variable();

                    if (def_var->valuetype() == nullptr) {
                        log && log("assign discovered lambda type T to enclosing define",
                                   xtag("lhs", def_var),
                                   xtag("T", print::unq(this->lambda_td_->canonical_name())));

                        def_var->assign_valuetype(lambda_td_);
                    } else {
                        /* don't need to unify here.  if def already hasa a type,
                         * that's because it was explicitly specified.
                         * will discover any conflict after reporting parsed lambda
                         * to define_xs
                         */
                    }
                }

                expect_expr_xs::start(p_psm);
                /* control reenters via .on_expr() or .on_expr_with_semicolon() */
            } else {
                this->illegal_input_on_type(c_self_name, td, this->get_expect_str(), p_psm);
            }
        }

        void
        lambda_xs::on_expr(bp<Expression> expr,
                           parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "lambda_xs::on_expr";

            if (lmxs_type_ == lambdastatetype::lm_4) {
                this->lmxs_type_ = lambdastatetype::lm_5;
                this->body_ = expr.promote();
            } else {
                this->illegal_input_on_expr(c_self_name, expr, this->get_expect_str(), p_psm);
            }
        }

        void
        lambda_xs::on_semicolon_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            if (lmxs_type_ == lambdastatetype::lm_5) {
                /* done! */

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                std::string name = Variable::gensym("lambda");

                /* top env frame recorded arguments to this lambda */
                p_psm->pop_envframe();

                rp<Lambda> lm;

                /* TODO: unify explicit_return_td_ with body_ */

                if (lambda_td_) {
                    lm = Lambda::make(name, lambda_td_, local_env_, body_);
                } else {
                    lm = Lambda::make_from_env(name, local_env_,
                                               explicit_return_td_, body_);
                }

                p_psm->top_exprstate().on_expr(lm, p_psm);
                p_psm->top_exprstate().on_semicolon_token(tk, p_psm);

                return;
            }

            exprstate::on_semicolon_token(tk, p_psm);
        }

        void
        lambda_xs::print(std::ostream & os) const {
            os << "<lambda_xs"
               << xtag("lmxs_type", lmxs_type_)
               << ">";
        }

        bool
        lambda_xs::pretty_print(const xo::print::ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "lambda_xs",
                                             refrtag("lmxs_type", lmxs_type_));
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end lambda_xs.cpp */
