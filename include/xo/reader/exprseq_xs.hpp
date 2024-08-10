/** @file exprseq_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        /** @class exprseq_xs
         *  @brief parsing state-machine for top-level expression sequence
         *
         **/
        class exprseq_xs : public exprstate {
        public:
            exprseq_xs();

            static std::unique_ptr<exprseq_xs> expect_toplevel_expression_sequence();

        public:
            // ----- token input methods -----

            virtual void on_def_token(const token_type & tk,
                                      exprstatestack * p_stack) override;
            virtual void on_symbol_token(const token_type & tk,
                                         exprstatestack * p_stack,
                                         rp<Expression> * p_emit_expr) override;

            // ----- victory methods -----

            virtual void on_typedescr(TypeDescr td,
                                      exprstatestack * p_stack,
                                      rp<Expression> * p_emit_expr) override;
            virtual void on_expr(ref::brw<Expression> expr,
                                 exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr) override;

        };
    } /*namespace scm*/
} /*namespace xo*/


/** end exprseq_xs.hpp **/
