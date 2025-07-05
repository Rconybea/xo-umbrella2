/** @file exprseq_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
//#include <cstdint>

namespace xo {
    namespace scm {
        enum class exprseqtype {
            /** toplevel interactive sequence.  Most permissive **/
            toplevel_interactive,
            /** toplevel non-interactive sequence.
             *  This used for top-level expressions in a translation unit.
             **/
            toplevel_batch,
            /** nested sequence, for example in function body **/
            nested,
        };

        /** @class exprseq_xs
         *  @brief parsing state-machine for top-level expression sequence
         *
         *  expression sequences come in several types:
         *  1. top-level interactive
         *  2. top-level batch
         *  3. nested
         *
         *           1 2 3
         *         +--------
         *  def    | y y y
         *  symbol | y n n         1: evaluate as variable
         *  i64    | y n n         1: evaluate as constant
         *
         **/
        class exprseq_xs : public exprstate {
        public:
            explicit exprseq_xs(exprseqtype seqtype);

            static void start(exprseqtype seqtype, parserstatemachine * p_psm);

        public:
            // ----- token input methods -----

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_i64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;

            // ----- victory methods -----

            virtual void on_typedescr(TypeDescr td,
                                      parserstatemachine * p_psm) override;
            virtual void on_expr(bp<Expression> expr,
                                 parserstatemachine * p_psm) override;
            virtual void on_expr_with_semicolon(bp<Expression> expr,
                                                parserstatemachine * p_psm) override;

        private:
            static std::unique_ptr<exprseq_xs> make(exprseqtype seqtype);

            /** context for this expression sequence **/
            exprseqtype xseqtype_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end exprseq_xs.hpp **/
