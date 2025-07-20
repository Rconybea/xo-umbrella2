/* file parserstatemachine.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "envframestack.hpp"
#include "parser_result.hpp"

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
            using LocalEnv = xo::ast::LocalEnv;
            using token_type = token<char>;

        public:
            parserstatemachine(exprstatestack * p_stack,
                               envframestack * p_env_stack,
                               parser_result * p_result)
                : p_stack_{p_stack},
                  p_env_stack_{p_env_stack},
                  p_result_{p_result}
                {}

            //const parser_result & result() const { return result_; }
            //parser_result_state result_state() const { return result_state_; }
            //const rp<Expression> & result_expr() const { return result_expr_; }
            //const char * error_src_function() const { return error_src_function_; }
            //const std::string & error_description() const { return error_description_; }

            std::unique_ptr<exprstate> pop_exprstate();
            exprstate & top_exprstate();
            void push_exprstate(std::unique_ptr<exprstate> x);

            /** lookup variable name in lexical context represented by
             *  this psm.  nullptr if not found
             **/
            bp<Variable> lookup_var(const std::string & x) const;

            /** update/replace binding for variable @p x in lexical context
             *  represented by this psm.  Always acts on innermost frame.
             **/
            void upsert_var(bp<Variable> x);

            bp<LocalEnv> top_envframe() const;
            void push_envframe(const rp<LocalEnv> & x);
            rp<LocalEnv> pop_envframe();
            size_t env_stack_size() const { return  p_env_stack_->size(); }

            // ----- parsing outputs -----

            void on_expr(bp<Expression> expr);
            void on_expr_with_semicolon(bp<Expression> expr);
            void on_symbol(const std::string & symbol);

            // ----- parsing inputs -----

            void on_semicolon_token(const token_type & tk);
            void on_operator_token(const token_type & tk);
            void on_leftbrace_token(const token_type & tk);
            void on_rightbrace_token(const token_type & tk);

            // ----- parsing error -----

            /** @p self_name  location (implementation function) where error detected
             **/
            void on_error(const char * self_name, std::string error_description);

            /** write human-readable representation on @p os **/
            void print(std::ostream & os) const;

        public:
            /** stack of incomplete parser work.
             *  generally speaking, push when to start new work for nested content;
             *  pop when work complete
             **/
            exprstatestack * p_stack_ = nullptr;
            /** stack of environment frames, one for each enclosing lambda **/
            envframestack * p_env_stack_ = nullptr;
            /** parser result object **/
            parser_result * p_result_ = nullptr;
        };

        inline std::ostream &
        operator<<(std::ostream & os, const parserstatemachine & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/
