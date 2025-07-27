/** @file lambda_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
#include "xo/expression/LocalEnv.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        /**
         * @text
         *
         *   lambda ( name(1) : type(1), ..., ) : type body-expr ;
         *  ^      ^                           ^ ^    ^         ^
         *  |      |                           | |    lm_4      lm_5
         *  |      |                           | lm_3
         *  lm_0   lm_1                        lm_2
         *
         *  lm_0 --on_lambda_token()--> lm_1
         *  lm_1 --on_formal_arglist()--> lm_2
         *  lm_2 --on_expr()--> lm_3
         *  lm_5 --on_semicolon_token()--> (done)
         *
         * @endtext
         **/
        enum class lambdastatetype {
            invalid = -1,

            lm_0,
            lm_1,
            lm_2,
            lm_3,
            lm_4,
            lm_5,

            n_lambdastatetype
        };

        extern const char *
        lambdastatetype_descr(lambdastatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, lambdastatetype x) {
            os << lambdastatetype_descr(x);
            return os;
        }

        /** @class lambda_xs
         *  @brief parsing state-machine for a lambda-expression
         *
         **/
        class lambda_xs : public exprstate {
        public:
            using Environment = xo::ast::Environment;
            using LocalEnv = xo::ast::LocalEnv;

        public:
            lambda_xs();

            static void start(parserstatemachine * p_psm);

            virtual const char * get_expect_str() const override;

            virtual void on_lambda_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;
            virtual void on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                           parserstatemachine * p_psm) override;
            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm) override;
            virtual void on_leftbrace_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_colon_token(const token_type & tk,
                                        parserstatemachine * p_psm) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;
            virtual bool pretty_print(const print::ppindentinfo & ppii) const override;

        private:
            static std::unique_ptr<lambda_xs> make();

        private:
            /** parsing state-machine state **/
            lambdastatetype lmxs_type_ = lambdastatetype::lm_0;

            /** lambda environment (for formal parameters) **/
            rp<LocalEnv> local_env_;

            /** explicit return type (if supplied) **/
            TypeDescr explicit_return_td_ = nullptr;

            /** body expression **/
            rp<Expression> body_;

            /** parent environment **/
            rp<Environment> parent_env_;

        };
    } /*namespace scm*/

    namespace print {
        PPDETAIL_ATOMIC(xo::scm::lambdastatetype);
    }
} /*namespace xo*/


/** end lambda_xs.hpp **/
