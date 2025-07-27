/* @file define_xs.cpp */

#include "define_xs.hpp"
#include "parserstatemachine.hpp"
#include "expect_symbol_xs.hpp"
#include "expect_expr_xs.hpp"
#include "expect_type_xs.hpp"
#include "pretty_expression.hpp"

namespace xo {
    namespace scm {
        // ----- defexprstatetype -----

        const char *
        defexprstatetype_descr(defexprstatetype x) {
            switch (x) {
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
            scope log(XO_DEBUG(p_psm->debug_flag()));

            p_psm->push_exprstate(define_xs::make());
            p_psm->top_exprstate().on_def_token(token_type::def(), p_psm);
        }

        define_xs::define_xs(rp<DefineExprAccess> def_expr)
            : exprstate(exprstatetype::defexpr),
              defxs_type_{defexprstatetype::def_0},
              def_expr_{std::move(def_expr)}
        {}

        const char *
        define_xs::get_expect_str() const
        {
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
            switch (this->defxs_type_) {
            case defexprstatetype::invalid:
            case defexprstatetype::def_0:
            case defexprstatetype::n_defexprstatetype:
                assert(false);  // impossible
                return nullptr;
            case defexprstatetype::def_1:
                return "symbol";
            case defexprstatetype::def_2:
                return "singleassign|colon";
            case defexprstatetype::def_4:
                return "singleassign";
            case defexprstatetype::def_3:
                return "type";
            case defexprstatetype::def_5:
                return "expression";
            case defexprstatetype::def_6:
                return "semicolon";
            }

            return "?expect";
        }

        void
        define_xs::on_expr(bp<Expression> expr,
                           parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

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

                if (this->cvt_expr_) {
                    this->cvt_expr_->assign_arg(rhs_value);
                } else {
                    /* note: establishes .def_expr_ valuetype */
                    this->def_expr_->assign_rhs(rhs_value);
                }

                rp<Expression> def_expr = this->def_expr_;

                this->defxs_type_ = defexprstatetype::def_6;
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_expr";
            const char * exp = get_expect_str();

            this->illegal_input_on_expr(c_self_name, expr, exp, p_psm);
        }

        void
        define_xs::on_expr_with_semicolon(bp<Expression> expr,
                                          parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("defxs_type", defxs_type_));

            this->on_expr(expr, p_psm);
            /* semicolon is allowed to terminate def expr */
            this->on_semicolon_token(token_type::semicolon(), p_psm);
        }

        void
        define_xs::on_symbol(const std::string & symbol_name,
                             parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("defxs_type", defxs_type_), xtag("env_stack_size", p_psm->env_stack_size()));

            if (this->defxs_type_ == defexprstatetype::def_1) {
                this->defxs_type_ = defexprstatetype::def_2;
                this->def_expr_->assign_lhs_name(symbol_name);

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
                    p_psm->upsert_var(this->def_expr_->lhs_variable());
                }

                return;
            }

            constexpr const char * c_self_name = "define_xs::on_symbol";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_symbol(c_self_name, symbol_name, exp, p_psm);
        }

        void
        define_xs::on_typedescr(TypeDescr td,
                                parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_3) {
                this->defxs_type_ = defexprstatetype::def_4;
                this->cvt_expr_ = ConvertExprAccess::make(td /*dest_type*/,
                                                          nullptr /*source_expr*/);
                /* note: establishes .def_expr_ valuetype */
                this->def_expr_->assign_rhs(this->cvt_expr_);
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_symbol";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_type(c_self_name, td, exp, p_psm);
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
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_def_token";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
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
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_symbol";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::on_semicolon_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            /* def expr consumes semicolon */

            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log("defxs_type", defxs_type_);

            if (this->defxs_type_ == defexprstatetype::def_6) {
                rp<DefineExprAccess> def_expr = this->def_expr_;

                std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

                p_psm->top_exprstate().on_expr(def_expr, p_psm);
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_symbol";
            const char * exp = this->get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::on_singleassign_token(const token_type & tk,
                                         parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log("defxs_type", defxs_type_);

            if ((this->defxs_type_ == defexprstatetype::def_2)
                || (this->defxs_type_ == defexprstatetype::def_4))
            {
                this->defxs_type_ = defexprstatetype::def_5;
                expect_expr_xs::start(p_psm);
                return;
            }

            constexpr const char * c_self_name = "define_xs::on_singleassign_token";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::on_rightparen_token(const token_type & tk,
                                       parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "define_xs::on_rightparen";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::on_i64_token(const token_type & tk,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "define_xs::on_i64";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::on_f64_token(const token_type & tk,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * c_self_name = "define_xs::on_f64";
            const char * exp = get_expect_str();

            this->illegal_input_on_token(c_self_name, tk, exp, p_psm);
        }

        void
        define_xs::print(std::ostream & os) const {
            os << "<define_xs"
               << xtag("defxs_type", defxs_type_);
            os << ">";
        }

        bool
        define_xs::pretty_print(const xo::print::ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "define_xs",
                                             refrtag("defxs_type", defxs_type_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end define_xs.cpp */
