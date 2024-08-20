/* @file sequence_xs.cpp */

#include "sequence_xs.hpp"
#include "parserstatemachine.hpp"
#include "expect_expr_xs.hpp"
#include "xo/expression/Sequence.hpp"

namespace xo {
    using xo::ast::Sequence;

    namespace scm {
        std::unique_ptr<sequence_xs>
        sequence_xs::make() {
            return std::make_unique<sequence_xs>(sequence_xs());
        }

        void
        sequence_xs::start(parserstatemachine * p_psm) {
            p_psm->push_exprstate(sequence_xs::make());
            /* want to accept anything that starts an expression,
             * except that } ends it
             */
            expect_expr_xs::start(true /*allow_defs*/,
                                  true /*cxl_on_rightbrace*/,
                                  p_psm);
        }

        sequence_xs::sequence_xs()
            : exprstate(exprstatetype::sequenceexpr)
        {}

        void
        sequence_xs::on_expr(ref::brw<Expression> expr,
                             parserstatemachine * p_psm)
        {
            /* TODO: if expr is a DefineExpr,
             *       then need to rewrite...
             *
             *   ...prefix
             *   DefineExpr(lhs_name, rhs)
             *   rest...
             *
             * becomes:
             *
             *   Sequence(
             *     ...prefix,
             *     Apply(Lambda(gen999, [Variable(lhs_name, type(rhs))], sequencify(rest...)),
             *           rhs))
             *
             * so amongst other things,
             * helpful to have nested seequence_xs that propagates '}' instead of swallowing it.
             */

            this->expr_v_.push_back(expr.promote());
            expect_expr_xs::start(true /*allow_defs*/,
                                  true /*cxl_on_rightbrace*/,
                                  p_psm);
        }

        void
        sequence_xs::on_rightbrace_token(const token_type & /*tk*/,
                                         parserstatemachine * p_psm)
        {
            auto self = p_psm->pop_exprstate();

            /* make sequence from expressions seen at this level,
             * and report it to parent
             */
            auto expr = Sequence::make(this->expr_v_);

            p_psm->top_exprstate().on_expr(expr, p_psm);
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end sequence_xs.cpp */
