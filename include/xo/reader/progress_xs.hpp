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
        /* represent an infix operator */
        enum class optype {
            invalid = -1,

            op_add,
            op_subtract,
            op_multiply,
            op_divide,

            n_optype
        };

        extern const char *
        optype_descr(optype x);

        inline std::ostream &
        operator<< (std::ostream & os, optype x) {
            os << optype_descr(x);
            return os;
        }

        /** @class progress_xs
         *  @brief state machine for parsing a schematica runtime-value-expression
         **/
        class progress_xs : public exprstate {
        public:
            progress_xs(rp<Expression> valex);
            virtual ~progress_xs() = default;

            static const progress_xs * from(const exprstate * x) { return dynamic_cast<const progress_xs *>(x); }

            static std::unique_ptr<progress_xs> make(rp<Expression> valex);

            bool admits_f64() const;

            virtual void on_expr(ref::brw<Expression> expr,
                                 exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr) override;
            virtual void on_symbol_token(const token_type & tk,
                                         exprstatestack * p_stack,
                                         rp<Expression> * p_emit_expr) override;
            virtual void on_typedescr(TypeDescr td,
                                      exprstatestack * /*p_stack*/,
                                      rp<Expression> * /*p_emit_expr*/) override;

            virtual void on_def_token(const token_type & tk,
                                      exprstatestack * p_stack) override;
            virtual void on_colon_token(const token_type & tk,
                                        exprstatestack * p_stack) override;
            virtual void on_semicolon_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_singleassign_token(const token_type & tk,
                                               exprstatestack * p_stack) override;
            virtual void on_leftparen_token(const token_type & tk,
                                            exprstatestack * p_stack,
                                            rp<Expression> * /*p_emit_expr*/) override;
            virtual void on_rightparen_token(const token_type & tk,
                                             exprstatestack * p_stack,
                                             rp<Expression> * /*p_emit_expr*/) override;

            /* entry point for an infix operator token */
            virtual void on_operator_token(const token_type & tk,
                                           exprstatestack * p_stack,
                                           rp<Expression> * p_emit_expr) override;

            virtual void on_f64_token(const token_type & tk,
                                      exprstatestack * p_stack,
                                      rp<Expression> * /*p_emit_expr*/) override;

            virtual void print(std::ostream & os) const override;

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
} /*namespace xo*/


/** end progress_xs.hpp **/
