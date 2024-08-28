/* file parserstatemachine.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "envframestack.hpp"

namespace xo {
    namespace scm {
        /** @class parserstatemachine
         *  @brief public parser state.
         *
         *  Schematica parser state; sent to subsidiary single-feature state machines.
         *  For example entry points for the lambda feature (@ref lambda_xs)
         *  will accept a non-const parserstatemachine pointer argument
         **/
        class parserstatemachine {
        public:
            using Expression = xo::ast::Expression;
            using Variable = xo::ast::Variable;
            using token_type = token<char>;

        public:
            parserstatemachine(exprstatestack * p_stack,
                               envframestack * p_env_stack,
                               rp<Expression> * p_emit_expr)
                : p_stack_{p_stack},
                  p_env_stack_{p_env_stack},
                  p_emit_expr_{p_emit_expr} {}

            std::unique_ptr<exprstate> pop_exprstate();
            exprstate & top_exprstate();
            void push_exprstate(std::unique_ptr<exprstate> x);

            /** lookup variable name in lexical context represented by
             *  this psm.  nullptr if not found
             **/
            rp<Variable> lookup_var(const std::string & x) const;

            void push_envframe(envframe x);
            void pop_envframe();

            // ----- parsing outputs -----

            void on_expr(ref::brw<Expression> expr);
            void on_expr_with_semicolon(ref::brw<Expression> expr);
            void on_symbol(const std::string & symbol);

            // ---- parsing inputs -----

            void on_semicolon_token(const token_type & tk);
            void on_operator_token(const token_type & tk);
            void on_leftbrace_token(const token_type & tk);
            void on_rightbrace_token(const token_type & tk);

            /** write human-readable representation on @p os **/
            void print(std::ostream & os) const;

        public:
            /** stack of incomplete parser work.
             *  generally speaking, push when to start new work for nested content;
             *  pop when work complete
             **/
            exprstatestack * p_stack_;
            /** stack of environment frames, one for each enclosing lambda **/
            envframestack * p_env_stack_;
            /** if non-null,  store next non-nested complete expressions in
             *  *p_emit_expr
             **/
            rp<Expression> * p_emit_expr_;
        };

        inline std::ostream &
        operator<<(std::ostream & os, const parserstatemachine & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.hpp */
