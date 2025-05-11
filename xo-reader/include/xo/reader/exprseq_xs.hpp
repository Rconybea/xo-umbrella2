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

            static void start(parserstatemachine * p_psm);

        public:
            // ----- token input methods -----

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;

            // ----- victory methods -----

            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;
            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;

        private:
            static std::unique_ptr<exprseq_xs> make();
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end exprseq_xs.hpp **/
