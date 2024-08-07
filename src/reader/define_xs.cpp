/* @file define_xs.cpp */

#include "define_xs.hpp"

namespace xo {
    namespace scm {
        define_xs::define_xs(rp<DefineExprAccess> def_expr)
            : exprstate(exprstatetype::def_0,
                        nullptr /*gen_expr*/,
                        def_expr)
        {}

        void
        define_xs::on_expr(ref::brw<Expression> expr,
                           exprstatestack * /* p_stack */,
                           rp<Expression> * /* p_emit_expr */)
        {
            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                assert(false);
                return;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
                /* NOT IMPLEMENTED */
                assert(false);
                return;
            case exprstatetype::def_4: {
                /* have all the ingredients to create an expression
                 * representing a definition
                 *
                 * 1. if ir_type is a symbol,  interpret as variable name.
                 *    Need to be able to locate variable by type
                 * 2. if ir_type is an expression,  adopt as rhs
                 */
                rp<Expression> rhs_value = expr.promote();

                if (this->cvt_expr_)
                    this->cvt_expr_->assign_arg(rhs_value);
                else
                    this->def_expr_->assign_rhs(rhs_value);;

                rp<Expression> def_expr = this->def_expr_;

                this->exs_type_ = exprstatetype::def_5;
                return;
            }

            case exprstatetype::def_5:
                assert(false);
                return;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_type:
            case exprstatetype::expect_symbol:
            case exprstatetype::expr_progress:
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end define_xs.cpp */
