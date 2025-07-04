/* @file exprseq_xs.cpp */

#include "exprseq_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "define_xs.hpp"
#include "expect_symbol_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<exprseq_xs>
        exprseq_xs::make()
        {
            return std::make_unique<exprseq_xs>(exprseq_xs());
        }

        void
        exprseq_xs::start(parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(exprseq_xs::make());
        }

        exprseq_xs::exprseq_xs()
            : exprstate(exprstatetype::expect_toplevel_expression_sequence)
        {
        }

        void
        exprseq_xs::on_def_token(const token_type & /*tk*/,
                                 parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * c_self_name = "exprseq_xs::on_def_token";

            define_xs::start(p_psm);

            /* keyword 'def' introduces a definition:
             *   def pi : f64 = 3.14159265
             *   def sq(x : f64) -> f64 { (x * x) }
             */
        }

        void
        exprseq_xs::on_symbol_token(const token_type & tk,
                                    parserstatemachine * /*p_psm*/)
        {
            constexpr const char * c_self_name = "exprseq_xs::on_symbol_token";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        exprseq_xs::on_typedescr(TypeDescr /*td*/,
                                 parserstatemachine * /*p_psm*/)
        {
            /* unreachable - typedescr should never get delivered to exprseq */
            assert(false);
            return;
        }

        void
        exprseq_xs::on_expr(ref::brw<Expression> expr,
                            parserstatemachine * p_psm)
        {
            /* toplevel expression sequence accepts an
             * arbitrary number of expressions.
             */

            auto p_emit_expr = p_psm->p_emit_expr_;

            *p_emit_expr = expr.promote();
        } /*on_expr*/

    } /*namespace scm*/
} /*namespace xo*/

/* end exprseq_xs.cpp */
