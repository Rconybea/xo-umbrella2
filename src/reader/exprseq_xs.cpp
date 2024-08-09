/* @file exprseq_xs.cpp */

#include "exprseq_xs.hpp"
#include "define_xs.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<exprseq_xs>
        exprseq_xs::expect_toplevel_expression_sequence()
        {
            return std::make_unique<exprseq_xs>(exprseq_xs());
        }

        exprseq_xs::exprseq_xs()
            : exprstate(exprstatetype::expect_toplevel_expression_sequence)
        {
        }

        void
        exprseq_xs::on_def_token(const token_type & /*tk*/,
                                 exprstatestack * p_stack)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * c_self_name = "exprseq_xs::on_def_token";

            p_stack->push_exprstate(define_xs::def_0());

            /* todo: replace:
             *   expect_symbol_or_function_signature()
             */
            p_stack->push_exprstate(exprstate::expect_symbol());

            /* keyword 'def' introduces a definition:
             *   def pi : f64 = 3.14159265
             *   def sq(x : f64) -> f64 { (x * x) }
             */
        }

        void
        exprseq_xs::on_symbol_token(const token_type & tk,
                                    exprstatestack * /*p_stack*/,
                                    rp<Expression> * /*p_emit_expr*/)
        {
            constexpr const char * c_self_name = "exprseq_xs::on_symbol_token";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        exprseq_xs::on_typedescr(TypeDescr /*td*/,
                                 exprstatestack * /*p_stack*/,
                                 rp<Expression> * /*p_emit_expr*/)
        {
            /* unreachable - typedescr should never get delivered to exprseq */
            assert(false);
            return;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end exprseq_xs.cpp */
