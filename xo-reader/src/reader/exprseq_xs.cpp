/* @file exprseq_xs.cpp */

#include "exprseq_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "exprseq_xs.hpp"
//#include "expect_expr_xs.hpp"
#include "progress_xs.hpp"
#include "define_xs.hpp"
#include "expect_symbol_xs.hpp"
#include "xo/expression/Constant.hpp"

namespace xo {
    namespace scm {
        std::unique_ptr<exprseq_xs>
        exprseq_xs::make(exprseqtype seqtype)
        {
            return std::make_unique<exprseq_xs>(exprseq_xs(seqtype));
        }

        void
        exprseq_xs::start(exprseqtype seqtype, parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(exprseq_xs::make(seqtype));
        }

        exprseq_xs::exprseq_xs(exprseqtype x)
            : exprstate(exprstatetype::expect_toplevel_expression_sequence),
              xseqtype_{x}
        {
        }

        void
        exprseq_xs::on_def_token(const token_type & /*tk*/,
                                 parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            define_xs::start(p_psm);

            /* keyword 'def' introduces a definition:
             *   def pi : f64 = 3.14159265
             *   def sq(x : f64) -> f64 { (x * x) }
             */
        }

        void
        exprseq_xs::on_symbol_token(const token_type & tk,
                                    parserstatemachine * p_psm)
        {
            constexpr const char * c_self_name = "exprseq_xs::on_symbol_token";

            if (xseqtype_ == exprseqtype::toplevel_interactive)
            {
                /* In interactive session, allow top-level variable reference.
                 * This could be:
                 *   a;      // variable references
                 *   a = 1;  // single assignment
                 *   a == 1; // rhs expression
                 *   a + b;  // rhs expression
                 * Variable must have been defined!
                 */
                bp<Variable> var = p_psm->lookup_var(tk.text());

                if (var.get()) {
                    progress_xs::start(var.promote(), p_psm);
                } else {
                    this->unknown_variable_error(c_self_name, tk, p_psm);
                }
            } else {
                /* policy: don't allow variable references as toplevel expressions
                 * unless interactive session
                 */
                this->illegal_input_error(c_self_name, tk);
            }
        }

        void
        exprseq_xs::on_i64_token(const token_type & tk,
                                 parserstatemachine * p_psm)
        {
            using xo::ast::Constant;

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "exprseq_xs::on_i64_token";

            if (xseqtype_ == exprseqtype::toplevel_interactive)
            {
                progress_xs::start(Constant<int64_t>::make(tk.i64_value()), p_psm);
            } else {
                /* policy: don't allow literals as toplevel expressions
                 * unless interactive session.
                 */
                this->illegal_input_error(c_self_name, tk);
            }
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
        exprseq_xs::on_expr(bp<Expression> expr,
                            parserstatemachine * p_psm)
        {
            /* toplevel expression sequence accepts an
             * arbitrary number of expressions.
             */


            *(p_psm->p_result_) = parser_result::expression(expr.promote());
        }

        void
        exprseq_xs::on_expr_with_semicolon(bp<Expression> expr,
                                           parserstatemachine * p_psm)
        {
            /* toplevel expression sequence accepts an
             * arbitrary number of expressions.
             *
             * semicolons are sometimes mandatory to avoid ambiguity.
             */

            *(p_psm->p_result_) = parser_result::expression(expr.promote());
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end exprseq_xs.cpp */
