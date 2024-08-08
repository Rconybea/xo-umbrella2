/* @file define_xs.cpp */

#include "define_xs.hpp"

namespace xo {
    namespace scm {
        define_xs::define_xs(rp<DefineExprAccess> def_expr)
            : exprstate(exprstatetype::def_0,
                        nullptr /*gen_expr*/,
                        def_expr)
        {}

        bool
        define_xs::admits_definition() const
        {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* note for def_4:
                 * rhs could certainly be a function body that contains
                 * nested defines; but then immediately-enclosing-exprstate
                 * would be a block
                 */
                return false;
            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
            case exprstatetype::expr_progress:
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        define_xs::admits_symbol() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                return false;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
            case exprstatetype::expr_progress:
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

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

        void
        define_xs::on_symbol(const std::string & symbol_name,
                             exprstatestack * /*p_stack*/,
                             rp<Expression> * /*p_emit_expr*/)
        {
            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* unreachable */
                assert(false);
                return;
            case exprstatetype::def_0:
                this->exs_type_ = exprstatetype::def_1;
                this->def_expr_->assign_lhs_name(symbol_name);
                //this->def_lhs_symbol_ = symbol_name;

                return;
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* NOT IMPLEMENTED */
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
