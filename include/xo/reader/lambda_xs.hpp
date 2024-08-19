/** @file lambda_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        /**
         *   lambda ( name(1) : type(1), ..., ) body-expr ;
         *  ^      ^                           ^         ^
         *  |      |                           |         |
         *  lm_0   lm_1                        lm_2      lm_3
         *
         *  lm_0 --on_lambda_token()--> lm_1
         *  lm_1 --on_formal_arglist()--> lm_2
         *  lm_2 --on_expr()--> lm_3
         *  lm_3 --on_semicolon_token()--> (done)
         **/
        enum class lambdastatetype {
            invalid = -1,

            lm_0,
            lm_1,
            lm_2,
            lm_3,

            n_lambdastatetype
        };

        /** @class lambda_xs
         *  @brief parsing state-machine for a lambda-expression
         *
         **/
        class lambda_xs : public exprstate {
        public:
            lambda_xs();

            static void start(parserstatemachine * p_psm);

            virtual void on_lambda_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                           parserstatemachine * p_psm) override;
            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;

        private:
            static std::unique_ptr<lambda_xs> make();

        private:
            /** parsing state-machine state **/
            lambdastatetype lmxs_type_ = lambdastatetype::lm_0;

            /** formal parameter list **/
            std::vector<rp<Variable>> argl_;

            /** body expression **/
            rp<Expression> body_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end lambda_xs.hpp **/
