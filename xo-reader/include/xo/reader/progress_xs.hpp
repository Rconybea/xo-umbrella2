/** @file progress_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
#include <iostream>
//#include <cstdint>

namespace xo {
    namespace scm {
        /** represent an infix operator.
         *
         *  See @ref progress_xs::assemble_expr() for translation
         *  to Expression
         **/
        enum class optype {
            invalid = -1,

            op_assign,

            op_add,
            op_subtract,

            op_multiply,
            op_divide,

            n_optype
        };

        extern const char *
        optype_descr(optype x);

        /** report operator precedence.
         *  lowest operator precedence is 1
         **/
        extern int
        precedence(optype x);

        inline std::ostream &
        operator<< (std::ostream & os, optype x) {
            os << optype_descr(x);
            return os;
        }

        /** @class progress_xs
         *  @brief state machine for parsing a schematica runtime-value-expression
         *
         *  Handles an expression that produces a value, for example appearing on the
         *  right-hand side of a definition.
         *
         *  Deals with infix operators, handles operator precedence.
         *  Also generates argument-type-specific arithmetic expressions,
         *  for example using ``Apply::make_add2_f64()`` when adding floating-point numbers
         **/
        class progress_xs : public exprstate {
        public:
            progress_xs(rp<Expression> valex, optype op);
            virtual ~progress_xs() = default;

            static const progress_xs * from(const exprstate * x) {
                return dynamic_cast<const progress_xs *>(x);
            }

            static void start(rp<Expression> valex,
                              optype optype,
                              parserstatemachine * p_psm);
            static void start(rp<Expression> valex,
                              parserstatemachine * p_psm);

            bool admits_f64() const;

            virtual const char * get_expect_str() const override;

            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm) override;
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_colon_token(const token_type & tk,
                                        parserstatemachine * p_psm) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_singleassign_token(const token_type & tk,
                                               parserstatemachine * p_psm) override;
            virtual void on_leftparen_token(const token_type & tk,
                                            parserstatemachine * p_psm) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

            /* entry point for an infix operator token */
            virtual void on_operator_token(const token_type & tk,
                                           parserstatemachine * p_psm) override;

            virtual void on_i64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            virtual void on_f64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            virtual void print(std::ostream & os) const override;

        private:
            static std::unique_ptr<progress_xs> make(rp<Expression> valex,
                                                     optype optype = optype::invalid);

        private:
            /** assemble expression representing
             *  value of
             *  @code
             *    f(lhs_, rhs_)
             *  @endcode
             *
             *  where f determined by @ref op_type_
             **/
            rp<Expression> assemble_expr();

        private:
            /** populate an expression here, may be followed by an operator **/
            rp<Expression> lhs_;

            /** infix operator,  if supplied **/
            optype op_type_ = optype::invalid;

            /** populate an expression here, following infix operator */
            rp<Expression> rhs_;
        };
    } /*namespace scm*/

#ifndef ppdetail_atomic
    namespace print {
        PPDETAIL_ATOMIC(xo::scm::optype);
    }
#endif
} /*namespace xo*/


/** end progress_xs.hpp **/
