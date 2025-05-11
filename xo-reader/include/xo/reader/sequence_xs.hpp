/** @file sequence_xs.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "exprstate.hpp"
#include <vector>

namespace xo {
    namespace ast { class Sequence; }
    namespace ast { class Lambda; }

    namespace scm {
        class sequence_xs : public exprstate {
        public:
            using Sequence = xo::ast::Sequence;
            using Lambda = xo::ast::Lambda;

        public:
            /** start parsing a sequence-expr.
             *  input begins with first expression in the sequence.
             **/
            static void start(parserstatemachine * p_psm);

            virtual void on_expr(ref::brw<Expression> expr,
                                 parserstatemachine * p_psm) override;

            virtual void on_rightbrace_token(const token_type & tk,
                                             parserstatemachine * p_psm) override;

        private:
            sequence_xs();

            /** named ctor idiom **/
            static std::unique_ptr<sequence_xs> make();

        private:
            /** will build SequenceExpr from in-order contents of this vector **/
            std::vector<rp<Expression>> expr_v_;
        };
    } /*namespace scm*/
} /*namespace xo*/


/** end sequence_xs.hpp **/
