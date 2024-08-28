/* file expect_expr_xs.cpp
 *
 * author: Roland Conybeare
 */

#include "expect_expr_xs.hpp"
#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "lambda_xs.hpp"
#include "define_xs.hpp"
#include "paren_xs.hpp"
#include "sequence_xs.hpp"
#include "progress_xs.hpp"
#include "xo/expression/Lambda.hpp"
#include "xo/expression/Constant.hpp"

namespace xo {
    using xo::ast::Constant;

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
                              parserstatemachine * p_psm) {
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

        void
        expect_expr_xs::on_def_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            if (allow_defs_)
                define_xs::start(p_psm);
            else
                exprstate::on_def_token(tk, p_psm);
        }

        void
        expect_expr_xs::on_lambda_token(const token_type & /*tk*/,
                                        parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            lambda_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_leftparen_token(const token_type & /*tk*/,
                                           parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            //constexpr const char * self_name = "exprstate::on_leftparen";

            /* push lparen_0 to remember to look for subsequent rightparen. */
            paren_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_leftbrace_token(const token_type & /*tk*/,
                                           parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            /* push lparen_0 to remember to look for subsequent rightparen. */
            sequence_xs::start(p_psm);
        }

        void
        expect_expr_xs::on_rightbrace_token(const token_type & tk,
                                            parserstatemachine * p_psm)
        {
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
            /* todo: treat symbol as variable name */

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

            rp<Variable> var = p_psm->lookup_var(tk.text());

            if (!var) {
                throw std::runtime_error
                    (tostr("expect_expr_xs::on_symbol_token",
                           ": unknown symbol",
                           xtag("name", tk.text())));
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
            progress_xs::start(var, p_psm);

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
        expect_expr_xs::on_f64_token(const token_type & tk,
                                     parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

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
        expect_expr_xs::on_expr(ref::brw<Expression> expr,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr.promote()));

            std::unique_ptr<exprstate> self = p_psm->pop_exprstate();

            p_psm->top_exprstate().on_expr(expr, p_psm);
        } /*on_expr*/

    } /*namespace scm*/
} /*namespace xo*/

/* end expect_expr_xs.cpp */
