/* @file lambda_xs.cpp */

#include "lambda_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "expect_formal_arglist_xs.hpp"
#include "expect_expr_xs.hpp"
#include "expect_type_xs.hpp"
#include "xo/expression/Lambda.hpp"

namespace xo {
    using xo::ast::Lambda;
    using xo::ast::LocalEnv;

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

            if (lmxs_type_ == lambdastatetype::lm_3) {
                this->lmxs_type_ = lambdastatetype::lm_4;
                this->explicit_return_td_ = td;
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

                rp<Lambda> lm = Lambda::make_from_env(name, local_env_, explicit_return_td_, body_);

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

    } /*namespace scm*/
} /*namespace xo*/


/* end lambda_xs.cpp */
