/* @file define_xs.cpp */

#include "define_xs.hpp"
#include "exprstatestack.hpp"
#include "parserstatemachine.hpp"
#include "expect_symbol_xs.hpp"
#include "expect_expr_xs.hpp"
#include "expect_type_xs.hpp"

namespace xo {
    namespace scm {
        // ----- defexprstatetype -----

        const char *
        defexprstatetype_descr(defexprstatetype x) {
            switch(x) {
            case defexprstatetype::invalid: return "invalid";
            case defexprstatetype::def_0: return "def_0";
            case defexprstatetype::def_1: return "def_1";
            case defexprstatetype::def_2: return "def_2";
            case defexprstatetype::def_3: return "def_3";
            case defexprstatetype::def_4: return "def_4";
            case defexprstatetype::def_5: return "def_5";
            case defexprstatetype::def_6: return "def_6";
            case defexprstatetype::n_defexprstatetype: break;
            }

            return "???defexprstatetype";
        }

        std::ostream &
        operator<<(std::ostream & os, defexprstatetype x) {
            os << defexprstatetype_descr(x);
            return os;
        }

        // ----- define_xs -----

        std::unique_ptr<define_xs>
        define_xs::make() {
            return std::make_unique<define_xs>(define_xs(DefineExprAccess::make_empty()));
        }

        void
        define_xs::start(parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            p_psm->push_exprstate(define_xs::make());
            p_psm->top_exprstate().on_def_token(token_type::def(), p_psm);
        }

        define_xs::define_xs(rp<DefineExprAccess> def_expr)
            : exprstate(exprstatetype::defexpr),
              defxs_type_{defexprstatetype::def_0},
              def_expr_{std::move(def_expr)}
        {}

        void
        define_xs::on_expr(bp<Expression> expr,
                           parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("defxs_type", defxs_type_));

            if (this->defxs_type_ == defexprstatetype::def_5) {
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

                this->defxs_type_ = defexprstatetype::def_6;
            } else {
                exprstate::on_expr(expr, p_psm);
            }
        }

        void
        define_xs::on_expr_with_semicolon(bp<Expression> expr,
                                          parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("defxs_type", defxs_type_));

            this->on_expr(expr, p_psm);
            /* semicolon is allowed to terminate def expr */
            this->on_semicolon_token(token_type::semicolon(), p_psm);
        }

        void
        define_xs::on_symbol(const std::string & symbol_name,
                             parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_1) {
                this->defxs_type_ = defexprstatetype::def_2;
                this->def_expr_->assign_lhs_name(symbol_name);
                return;
            } else {
                exprstate::on_symbol(symbol_name, p_psm);
            }
        }

        void
        define_xs::on_typedescr(TypeDescr td,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_3) {
                this->defxs_type_ = defexprstatetype::def_4;
                this->cvt_expr_ = ConvertExprAccess::make(td /*dest_type*/,
                                                          nullptr /*source_expr*/);
                this->def_expr_->assign_rhs(this->cvt_expr_);
                //this->def_lhs_td_ = td;

                return;
            } else {
                exprstate::on_typedescr(td, p_psm);
            }
        }

        void
        define_xs::on_def_token(const token_type & tk,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_0) {
                this->defxs_type_ = defexprstatetype::def_1;

                expect_symbol_xs::start(p_psm);
            } else {
                exprstate::on_def_token(tk, p_psm);
            }
        }

        void
        define_xs::on_colon_token(const token_type & tk,
                                  parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_2) {
                this->defxs_type_ = defexprstatetype::def_3;

                expect_type_xs::start(p_psm);
            } else {
                exprstate::on_colon_token(tk, p_psm);
            }
        }

        void
        define_xs::on_semicolon_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            /* def expr consumes semicolon */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_6) {
                rp<DefineExprAccess> def_expr = this->def_expr_;

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                // if this is a genuine top-level define (i.e. nesting level = 0),
                // then we need to upsert so we can refer to rhs later.
                //
                // In other contexts (e.g. body-of-lambda) will be rewriting
                //    {
                //       def y = foo(x,x);
                //       bar(y,y);
                //    }
                // into something like
                //    {
                //       (lambda (y123) bar(y123,y123))(foo(x,x));
                //    }
                //
                // This works in the body of lambda, because we don't evaluate anything
                // until lambda definition is complete.
                //
                // For interactive top-level defs we want to evaluate as we go,
                // so need incremental bindings.

                if (p_psm->env_stack_size() == 1) {
                    /* remember variable binding in lexical context,
                     * so we can refer to it later
                     */
                    p_psm->upsert_var(def_expr->lhs_variable());
                }

                p_psm->top_exprstate().on_expr(def_expr, p_psm);
            } else {
                exprstate::on_semicolon_token(tk, p_psm);
            }
        }

        void
        define_xs::on_singleassign_token(const token_type & tk,
                                         parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "define_xs::on_singleassign_token";

            log && log("defxs_type", defxs_type_);

            /*
             *   def foo       = 1 ;
             *   def foo : f64 = 1 ;
             *  ^   ^   ^ ^   ^ ^ ^ ^
             *  |   |   | |   | | | (done)
             *  |   |   | |   | | def_6
             *  |   |   | |   | def_5:expect_rhs_expression
             *  |   |   | |   def_4
             *  |   |   | def_3:expect_type
             *  |   |   def_2
             *  |   def_1:expect_symbol
             *  expect_toplevel_expression_sequence
             *
             * note that we skip from def_2 -> def_5 if '=' instead of ':'
             */
            if ((this->defxs_type_ == defexprstatetype::def_2)
                || (this->defxs_type_ == defexprstatetype::def_4))
            {
                this->defxs_type_ = defexprstatetype::def_5;

                expect_expr_xs::start(p_psm);
            } else {
                this->illegal_input_error(self_name, tk);
            }
        }

        void
        define_xs::on_rightparen_token(const token_type & tk,
                                       parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "define_xs::on_rightparen";

            this->illegal_input_error(self_name, tk);
        }

        void
        define_xs::on_i64_token(const token_type & tk,
                                parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "define_xs::on_i64";

            this->illegal_input_error(self_name, tk);
        }

        void
        define_xs::on_f64_token(const token_type & tk,
                                parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "define_xs::on_f64";

            this->illegal_input_error(self_name, tk);
        }

        void
        define_xs::print(std::ostream & os) const {
            os << "<define_xs"
               << xtag("this", (void*)this)
                //<< xtag("type", exs_type_)
               << xtag("defxs_type", defxs_type_);

            //if (def_expr_)
            //    os << xtag("def_expr", def_expr_);
            //if (cvt_expr_)
            //    os << xtag("cvt_expr", cvt_expr_);
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end define_xs.cpp */
