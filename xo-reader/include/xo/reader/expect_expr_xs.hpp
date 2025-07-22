/* file expect_expr_xs.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"

namespace xo {
    namespace scm {

        /** @class expect_expr_xs
         *  @brief state machine to expect + capture an expression
         *
         *  Examples:
         *  @text
         *    def x : i64 = 5 ;           // with allow_defs
         *    lambda (f : x64) { ... } ;
         *    if (prime(x)) { ... } ;
         *    5 + x ;
         *  @endtext
         *
         *  top exprstate when expect_expr_xs::start() invoked
         *  will receive parsed expression via
         *  exprstate::on_expr() or exprstate::on_expr_with_semicolon().
         **/
        class expect_expr_xs : public exprstate {
        public:
            explicit expect_expr_xs(bool allow_defs,
                                    bool cxl_on_rightbrace);

            static void start(parserstatemachine * p_psm);
            static void start(bool allow_defs,
                              bool cxl_on_rightbrace,
                              parserstatemachine * p_psm);

            virtual void on_lambda_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            virtual void on_leftparen_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;

            virtual void on_leftbrace_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;

            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;

            virtual void on_bool_token(const token_type & tk,
                                       parserstatemachine * p_psm) override;

            virtual void on_i64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            virtual void on_f64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            /** update exprstate in response to a successfully-parsed subexpression **/
            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;
            /** update exprstate in response to a successfully-parsed subexpression
             *  that's terminated by semicolon ';'
             **/
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm) override;


        private:
            static std::unique_ptr<expect_expr_xs> make(bool allow_defs,
                                                        bool cxl_on_rightbrace);

        private:
            /* if true: allow a define-expression here */
            bool allow_defs_ = false;
            /* if true: expecting either:
             *   - expression
             *   - right brace '}',  in which case no expression
             * if false: expecting
             *   - expression
             */
            bool cxl_on_rightbrace_ = false;
        };

    } /*namespace scm*/
} /*namespace xo*/


/* end expect_expr_xs.hpp */
