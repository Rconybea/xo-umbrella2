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
        enum class exprstatetype {
            invalid = -1,

            /** toplevel of some translation unit **/
            expect_toplevel_expression_sequence,

            /** handle define-expression
             *  see @ref define_xs
             **/
            defexpr,

            /** handle parenthesized expression
             *  see @ref paren_xs
             **/
            parenexpr,

            expect_rhs_expression,
            expect_symbol,
            expect_type,

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

        class exprstatestack;

        class define_xs;

        /** state associated with a partially-parsed expression.
         **/
        class exprstate {
        public:
            using Expression = xo::ast::Expression;
            using exprtype = xo::ast::exprtype;
            using token_type = token<char>;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            exprstate() = default;
            exprstate(exprstatetype exs_type)
                : exs_type_{exs_type}
                {}
            virtual ~exprstate() = default;

            static std::unique_ptr<exprstate> expect_toplevel_expression_sequence() {
                return std::make_unique<exprstate>(exprstate(exprstatetype::expect_toplevel_expression_sequence));
            }
            static std::unique_ptr<exprstate> expect_rhs_expression() {
                return std::make_unique<exprstate>(exprstate(exprstatetype::expect_rhs_expression));
            }
            static std::unique_ptr<exprstate> expect_symbol() {
                return std::make_unique<exprstate>(exprstate(exprstatetype::expect_symbol));
            }
            static std::unique_ptr<exprstate> expect_type() {
                return std::make_unique<exprstate>(exprstate(exprstatetype::expect_type));
            }

            exprstatetype exs_type() const { return exs_type_; }

            /** true iff this parsing state admits a 'def' keyword
             *  as next token
             **/
            virtual bool admits_definition() const;
            /** true iff this parsing state admits a symbol as next token **/
            virtual bool admits_symbol() const;
            /** true iff this parsing state admits a colon as next token **/
            virtual bool admits_colon() const;
            /** true iff this parsing state admits a semicolon as next token **/
            virtual bool admits_semicolon() const;
            /** true iff this parsing state admits a singleassign '=' as next token **/
            virtual bool admits_singleassign() const;
            /** true iff this parsing state admits a leftparen '(' as next token **/
            virtual bool admits_leftparen() const;
            /** truee iff this parsing state admits a rightparen ')' as next token **/
            virtual bool admits_rightparen() const;
            /** true iff this parsing state admits a 64-bit floating point literal token **/
            virtual bool admits_f64() const;

            /** update exprstate in response to incoming token @p tk,
             *  forward instructions to parent parser
             **/
            void on_input(const token_type & tk, exprstatestack * p_stack, rp<Expression> * p_emit_expr);

            /** update exprstate in response to a successfully-parsed subexpression **/
            virtual void on_expr(ref::brw<Expression> expr,
                                 exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr);
            /** update exprstate when expecting a symbol **/
            virtual void on_symbol(const std::string & symbol,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr);
            /** update exprstate when expeccting a typedescr **/
            virtual void on_typedescr(TypeDescr td,
                                      exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr);
            /** print human-readable representation on @p os **/
            virtual void print(std::ostream & os) const;

            // ----- input methods -----

            virtual void on_def(exprstatestack * p_stack);
            /** handle incoming symbol token **/
            virtual void on_symbol_token(const token_type & tk,
                                         exprstatestack * p_stack,
                                         rp<Expression> * p_emit_expr);
            /** handle incoming ':' token **/
            virtual void on_colon_token(const token_type & tk,
                                        exprstatestack * p_stack);
            /** handle incoming ';' token **/
            virtual void on_semicolon_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * p_emit_expr);
            /** handle incoming '=' token **/
            virtual void on_singleassign_token(const token_type & tk,
                                               exprstatestack * p_stack);
            /** handle incoming '(' token **/
            virtual void on_leftparen_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr);
            /** handle incoming ')' token **/
            virtual void on_rightparen_token(const token_type & tk,
                                             exprstatestack * p_stack,
                                             rp<Expression> * p_emit_expr);
            /** handle incoming floating-point-literal token **/
            virtual void on_f64_token(const token_type & tk,
                                      exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr);

        protected:
            /** explicit subtype: identifies derived class **/
            exprstatetype exs_type_;
        }; /*exprstate*/

        inline std::ostream &
        operator<< (std::ostream & os, const exprstate & x) {
            x.print(os);
            return os;
        }

        /** @class exprstatestack
         *  @brief A stack of exprstate objects
         **/
        class exprstatestack {
        public:
            exprstatestack() {}

            bool empty() const { return stack_.empty(); }
            std::size_t size() const { return stack_.size(); }

            exprstate & top_exprstate();
            void push_exprstate(std::unique_ptr<exprstate> exs);
            std::unique_ptr<exprstate> pop_exprstate();

            /** relative to top-of-stack.
             *  0 -> top (last in),  z-1 -> bottom (first in)
             **/
            std::unique_ptr<exprstate> & operator[](std::size_t i) {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            const std::unique_ptr<exprstate> & operator[](std::size_t i) const {
                std::size_t z = stack_.size();

                assert(i < z);

                return stack_[z - i - 1];
            }

            void print (std::ostream & os) const;

        private:
            std::vector<std::unique_ptr<exprstate>> stack_;
        };

        inline std::ostream &
        operator<< (std::ostream & os, const exprstatestack & x) {
            x.print(os);
            return os;
        }

        inline std::ostream &
        operator<< (std::ostream & os, const exprstatestack * x) {
            x->print(os);
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/

/** end exprstate.hpp **/
