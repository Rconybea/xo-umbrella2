/** @file exprstate.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "xo/expression/Expression.hpp"
#include "xo/tokenizer/token.hpp"
#include <stack>
//#include <cstdint>

namespace xo {
    namespace scm {
        /** Identify current state in an expression state machine
         **/
        enum class exprstatetype {
            invalid = -1,

            /** toplevel of some translation unit **/
            expect_toplevel_expression_sequence,

            /** handle define-expression
             *  see @ref define_xs
             **/
            defexpr,

            /** handle lambda-expression
             *  see @ref lambda_xs
             **/
            lambdaexpr,

            /** handle parenthesized expression
             *  see @ref paren_xs
             **/
            parenexpr,

            /** handle sequence expression (aka block)
             *  see @ref sequence_xs
             **/
            sequenceexpr,

            /** handle let1 (single local variable)
             *  see @ref let1_xs
             **/
            let1expr,

            expect_rhs_expression,
            expect_symbol,
            expect_type,
            /** handle formal argument list
             *  see @ref expect_formal_arglist_xs
             **/
            expect_formal_arglist,
            /** handle formal argument
             *  see @ref expec_formal_xs
             **/
            expect_formal,

            /** handle expression-in-progress,
             *  in case infix operators to follow
             *  see @ref progress_xs
             **/
            expr_progress,

            n_exprstatetype
        };

        extern const char *
        exprstatetype_descr(exprstatetype x);

        inline std::ostream &
        operator<< (std::ostream & os, exprstatetype x) {
            os << exprstatetype_descr(x);
            return os;
        }

        class parserstatemachine; /* see parserstatemachine.hpp */
        class exprstatestack; /* see exprstatestack.hpp */

        class formal_arg;

        /** state associated with a partially-parsed expression.
         **/
        class exprstate {
        public:
            using Expression = xo::ast::Expression;
            using Variable = xo::ast::Variable;
            using exprtype = xo::ast::exprtype;
            using token_type = token<char>;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            exprstate() = default;
            exprstate(exprstatetype exs_type)
                : exs_type_{exs_type}
                {}
            virtual ~exprstate() = default;

            exprstatetype exs_type() const { return exs_type_; }

            /** update exprstate in response to incoming token @p tk,
             *  forward instructions to parent parser
             **/
            void on_input(const token_type & tk,
                          parserstatemachine * p_psm);

            /** update exprstate in response to a successfully-parsed subexpression **/
            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm);

            /** update exprstate in response to a successfully-parsed subexpression, that ends with semicolon **/
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm);

            /** update exprstate when expecting a symbol **/
            virtual void on_symbol(const std::string & symbol,
                                   parserstatemachine * p_psm);

            /** update exprstate when expeccting a typedescr **/
            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm);

            /** update exprstate when expecting a formal parameter **/
            virtual void on_formal(const rp<Variable> & formal,
                                   parserstatemachine * p_psm);

            /** update expression when epecting a formal parameter list **/
            virtual void on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                           parserstatemachine * p_psm);

            /** print human-readable representation on @p os **/
            virtual void print(std::ostream & os) const;

            // ----- input methods -----

            /** handle incoming 'def' token **/
            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm);
            /** handle incoming 'lambda' token **/
            virtual void on_lambda_token(const token_type & tk,
                                         parserstatemachine * p_psm);
            /** handle incoming symbol token **/
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm);
            /** handle incoming ',' token **/
            virtual void on_comma_token(const token_type & tk,
                                        parserstatemachine * p_psm);
            /** handle incoming ':' token **/
            virtual void on_colon_token(const token_type & tk,
                                        parserstatemachine * p_psm);
            /** handle incoming ';' token **/
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm);
            /** handle incoming '=' token **/
            virtual void on_singleassign_token(const token_type & tk,
                                               parserstatemachine * p_psm);
            /** handle incoming '(' token **/
            virtual void on_leftparen_token(const token_type & tk,
                                            parserstatemachine * p_psm);
            /** handle incoming ')' token **/
            virtual void on_rightparen_token(const token_type & tk,
                                             parserstatemachine * p_psm);

            /** handle incoming '{' token **/
            virtual void on_leftbrace_token(const token_type & tk,
                                            parserstatemachine * p_psm);

            /** handle incoming '}' token **/
            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm);

            /** handle incoming operator token **/
            virtual void on_operator_token(const token_type & tk,
                                           parserstatemachine * p_psm);

            /** handle incoming integer-literal token **/
            virtual void on_i64_token(const token_type & tk,
                                      parserstatemachine * p_psm);

            /** handle incoming floating-point-literal token **/
            virtual void on_f64_token(const token_type & tk,
                                      parserstatemachine * p_psm);

        protected:
            /** throw exception when next token is inconsistent with
             *  parsing state
             **/
            void illegal_input_error(const char * self_name,
                                     const token_type & tk) const;

            /** throw exception when unable to locate definition for a variable
             **/
            void unknown_variable_error(const char * self_name,
                                        const token_type & tk) const;

        protected:
            /** explicit subtype: identifies derived class **/
            exprstatetype exs_type_;
        }; /*exprstate*/

        inline std::ostream &
        operator<< (std::ostream & os, const exprstate & x) {
            x.print(os);
            return os;
        }

        inline std::ostream &
        operator<< (std::ostream & os, const exprstate * x) {
            if (x)
                x->print(os);
            else
                os << "nullptr";
            return os;
        };
    } /*namespace scm*/

#ifndef ppdetail_atomic
    namespace print {
        PPDETAIL_ATOMIC(xo::scm::exprstatetype);
        PPDETAIL_ATOMIC(xo::scm::exprstate);
    } /*namespace print*/
#endif
} /*namespace xo*/

/** end exprstate.hpp **/
