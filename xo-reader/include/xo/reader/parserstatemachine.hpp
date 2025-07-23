/* file parserstatemachine.hpp
 *
 * author: Roland Conybeare, Aug 2024
 */

#pragma once

#include "exprstate.hpp"
#include "exprstatestack.hpp"
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
            explicit parserstatemachine(bool debug_flag)
                : debug_flag_{debug_flag}
                {}

            //const parser_result & result() const { return result_; }
            //parser_result_state result_state() const { return result_state_; }
            //const rp<Expression> & result_expr() const { return result_expr_; }
            //const char * error_src_function() const { return error_src_function_; }
            //const std::string & error_description() const { return error_description_; }

            std::unique_ptr<exprstate> pop_exprstate();
            exprstate & top_exprstate();
            void push_exprstate(std::unique_ptr<exprstate> x);

            bool debug_flag() const { return debug_flag_; }

            /** lookup variable name in lexical context represented by
             *  this psm.  nullptr if not found
             **/
            bp<Variable> lookup_var(const std::string & x) const;

            /** update/replace binding for variable @p x in lexical context
             *  represented by this psm.  Always acts on innermost frame.
             **/
            void upsert_var(bp<Variable> x);

            /** @return available variable bindings in current parsing state **/
            bp<LocalEnv> top_envframe() const;
            /** push frame @p x (with new variable bindings) onto environment stack **/
            void push_envframe(const rp<LocalEnv> & x);
            /** @return pop innermost environment frame and return it **/
            rp<LocalEnv> pop_envframe();
            /** @return number of stacked environment frames **/
            size_t env_stack_size() const { return  env_stack_.size(); }

            // ----- parsing outputs -----

            void on_expr(bp<Expression> expr);
            void on_expr_with_semicolon(bp<Expression> expr);
            void on_symbol(const std::string & symbol);

            // ----- parsing inputs -----

            void on_semicolon_token(const token_type & tk);
            void on_operator_token(const token_type & tk);
            void on_leftbrace_token(const token_type & tk);
            void on_rightbrace_token(const token_type & tk);
            void on_then_token(const token_type & tk);
            void on_else_token(const token_type & tk);

            // ----- parsing error -----

            /** @p self_name  location (implementation function) where error detected
             **/
            void on_error(const char * self_name, std::string error_description);

            /** write human-readable representation on @p os **/
            void print(std::ostream & os) const;

        public:
            /** state recording state associated with enclosing expressions.
             *
             *  Note: at least asof c++23, the std::stack api doesn't support access
             *  to members other than the top.
             *
             *  for stack with N elements (N = stack_.size()):
             *  - bottom of stack is stack_[0]
             *  - top of stack is stack_[N-1]
             **/
            exprstatestack xs_stack_;
            /** environment frames for lexical context.
             *  push a frame on each nested lambda;
             *  pop when lambda body goes out of scope
             **/
            envframestack env_stack_;
            /** parser result state **/
            parser_result result_;
            /** enable/disable debug logging **/
            bool debug_flag_ = false;
        };

        inline std::ostream &
        operator<<(std::ostream & os, const parserstatemachine & x) {
            x.print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/
