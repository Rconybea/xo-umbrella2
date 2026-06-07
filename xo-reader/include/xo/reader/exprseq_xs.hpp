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
        };

        /** @class exprseq_xs
         *  @brief parsing state-machine for top-level expression sequence
         *
         *  expression sequences come in several types:
         *  1. top-level interactive
         *  2. top-level batch
         *
         *  @text
         *           1 2
         *         +-----
         *  def    | y y
         *  if     | y n     1: eval as expression
         *  symbol | y n     1: evaluate as variable
         *  i64    | y n     1: evaluate as constant
         *
         *  @endtext
         **/
        class exprseq_xs : public exprstate {
        public:
            explicit exprseq_xs(exprseqtype seqtype);

            static void start(exprseqtype seqtype, parserstatemachine * p_psm);

        public:
            virtual const char * get_expect_str() const override;

            // ----- token input methods -----

            virtual void on_def_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_if_token(const token_type & tk,
                                     parserstatemachine * p_psm) override;
            virtual void on_symbol_token(const token_type & tk,
                                         parserstatemachine * p_psm) override;
            virtual void on_bool_token(const token_type & tk,
                                       parserstatemachine * p_psm) override;
            virtual void on_i64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_f64_token(const token_type & tk,
                                      parserstatemachine * p_psm) override;
            virtual void on_string_token(const token_type & tk,
                                         parserstatemachine * p_psm) final override;

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
