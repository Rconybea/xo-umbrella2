/* file expect_expr_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "define_xs.hpp"
#include "lambda_xs.hpp"
#include "if_else_xs.hpp"
#include "paren_xs.hpp"
#include "sequence_xs.hpp"
#include "progress_xs.hpp"
#include "xo/expression/Lambda.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/pretty_expression.hpp"

namespace xo {
    using xo::scm::Constant;

    namespace scm {

        std::unique_ptr<expect_expr_xs>
        expect_expr_xs::make(bool allow_defs,
                             bool cxl_on_rightbrace)
        {
            return std::make_unique<expect_expr_xs>(expect_expr_xs(allow_defs,
                                                                   cxl_on_rightbrace));

        }

        void
        expect_expr_xs::start(bool allow_defs,
                              bool cxl_on_rightbrace,
                              parserstatemachine * p_psm)
        {
            p_psm->push_exprstate(expect_expr_xs::make(allow_defs,
                                                       cxl_on_rightbrace));
        }

        void
        expect_expr_xs::start(parserstatemachine * p_psm) {
            start(false /*!allow_defs*/,
                  false /*!cxl_on_rightbrace*/,
                  p_psm);
        }

        expect_expr_xs::expect_expr_xs(bool allow_defs,
                                       bool cxl_on_rightbrace)
            : exprstate(exprstatetype::expect_rhs_expression),
              allow_defs_{allow_defs},
              cxl_on_rightbrace_{cxl_on_rightbrace}
        {}

        const char *
        expect_expr_xs::get_expect_str() const
        {
            if (allow_defs_) {
                return "def|lambda|lparen|lbrace|literal|var";
            } else {
                return "lambda|lparen|lbrace|literal|var";
            }
        }

        void
        expect_expr_xs::on_def_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            if (allow_defs_) {
                define_xs::start(p_psm);
            } else {
                exprstate::on_def_token(tk, p_psm);
            }
        }

        void
        expect_expr_xs::on_lambda_token(const token_type & /*tk*/,
                                        parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            lambda_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_if_token(const token_type & /*tk*/,
                                    parserstatemachine * p_psm)
        {
            if_else_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_leftparen_token(const token_type & /*tk*/,
                                           parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            /* push lparen_0 to remember to look for subsequent rightparen. */
            paren_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_leftbrace_token(const token_type & /*tk*/,
                                           parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            /* push lparen_0 to remember to look for subsequent rightparen. */
            sequence_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_rightbrace_token(const token_type & tk,
                                            parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            if (cxl_on_rightbrace_) {
                auto self = p_psm->pop_exprstate();

                /* do not call .on_expr(), since '}' cancelled */

                p_psm->on_rightbrace_token(tk);
            } else {
                exprstate::on_rightbrace_token(tk, p_psm);
            }
        }

        void
        expect_expr_xs::on_symbol_token(const token_type & tk,
                                        parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("tk", tk));

            constexpr const char * c_self_name = "expect_expr_xs::on_symbol_token";

            /* various possibilities when looking for rhs expression:
             *
             *   x := y       // (1)
             *   x := f(a)    // (2)
             *   x := f(a,b)  // (3)
             *
             * need lookahead token following symbol to distinguish
             * between (1) (symbol completes rhs expression)
             * and {(2), (3)} (symbol is function call)
             */

            bp<Variable> var = p_psm->lookup_var(tk.text());

            if (!var) {
                this->unknown_variable_error(c_self_name, tk, p_psm);
                return;
            }

            /* e.g.
             *   def pi = 3.14159265;
             *   def mypi = pi;
             *              ^
             *   def pi2 = pi * 2;
             *             ^
             *   def y = foo(pi2);
             *           ^
             */
            progress_xs::start(var.promote(), p_psm);

#ifdef NOT_YET
            p_stack->push_exprstate(exprstate(exprstatetype::expr_progress,
                                              Variable::make(name, type)));
#endif

#ifdef LATER
            p_psm->pop_exprstate();
            p_psm->top_exprstate().on_symbol(tk.text(),
                                               p_stack, p_emit_expr);
#endif
            return;
        }

        void
        expect_expr_xs::on_bool_token(const token_type & tk,
                                      parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            progress_xs::start
                (Constant<bool>::make(tk.bool_value()),
                 p_psm);
        }

        void
        expect_expr_xs::on_i64_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            progress_xs::start
                (Constant<int64_t>::make(tk.i64_value()),
                 p_psm);
        }

        void
        expect_expr_xs::on_f64_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            //constexpr const char * self_name = "exprstate::on_f64_token";

            /* e.g.
             *   def pi = 3.14159265;
             *            \---tk---/
             */
            progress_xs::start
                (Constant<double>::make(tk.f64_value()),
                 p_psm);
        }

        void
        expect_expr_xs::on_expr(bp<Expression> expr,
                                parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr.promote()));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr(expr);
        } /*on_expr*/

        void
        expect_expr_xs::on_expr_with_semicolon(bp<Expression> expr,
                                               parserstatemachine * p_psm)
        {
            scope log(XO_DEBUG(p_psm->debug_flag()));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr.promote()));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->on_expr_with_semicolon(expr);
        } /*on_expr_with_semicolon*/

        void
        expect_expr_xs::print(std::ostream & os) const {
            os << "<expect_expr_xs"
               << xtag("allow_defs", allow_defs_)
               << xtag("cxl_on_rightbrace", cxl_on_rightbrace_)
               << ">";
        }

        bool
        expect_expr_xs::pretty_print(const xo::print::ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "expect_expr_xs",
                                             refrtag("allow_defs", allow_defs_),
                                             refrtag("cxl_on_rightbrace", cxl_on_rightbrace_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end expect_expr_xs.cpp */
