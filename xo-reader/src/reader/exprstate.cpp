/* @file exprstate.cpp */

#include "exprstate.hpp"
#include "exprstatestack.hpp"
#include "parserstatemachine.hpp"
#include "pretty_parserstatemachine.hpp"
#include "pretty_expression.hpp"
//#include "formal_arg.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/indentlog/print/vector.hpp"
#include "xo/indentlog/print/pretty_vector.hpp"
#include <stdexcept>
//#include "define_xs.hpp"
//#include "progress_xs.hpp"
//#include "paren_xs.hpp"
//#include "expect_expr_xs.hpp"
//#include "xo/expression/Constant.hpp"
//#include "xo/reflect/Reflect.hpp"

namespace xo {
    //using xo::ast::Constant;
    //using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        const char *
        exprstatetype_descr(exprstatetype x) {
            switch (x) {
            case exprstatetype::invalid:
                return "?invalid";
            case exprstatetype::expect_toplevel_expression_sequence:
                return "expect_toplevel_expression_sequence";
            case exprstatetype::defexpr:
                return "defexpr";
            case exprstatetype::lambdaexpr:
                return "lambdaexpr";
            case exprstatetype::parenexpr:
                return "parenexpr";
            case exprstatetype::sequenceexpr:
                return "sequenceexpr";
            case exprstatetype::let1expr:
                return "let1expr";
            case exprstatetype::expect_rhs_expression:
                return "expect_rhs_expression";
            case exprstatetype::expect_symbol:
                return "expect_symbol";
            case exprstatetype::expect_type:
                return "expect_type";
            case exprstatetype::expect_formal_arglist:
                return "expect_formal_arglist";
            case exprstatetype::expect_formal:
                return "expect_formal";
            case exprstatetype::expr_progress:
                return "expr_progress";
            case exprstatetype::n_exprstatetype:
                break;
            }

            return "???";
        }

        void
        exprstate::on_def_token(const token_type & tk,
                                parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", p_psm->top_exprstate().exs_type()));

            this->illegal_input_error("exprstate::on_def_token", tk);
        }

        void
        exprstate::on_lambda_token(const token_type & tk,
                                   parserstatemachine * /*p_psm*/)
        {
            this->illegal_input_error("exprstate::on_lambda_token", tk);
        }

        void
        exprstate::on_symbol_token(const token_type & tk,
                                   parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", p_psm->top_exprstate().exs_type()));

            constexpr const char * c_self_name = "exprstate::on_symbol_token";

            this->illegal_input_error(c_self_name, tk);
        }

        void
        exprstate::on_typedescr(TypeDescr td,
                                parserstatemachine * p_psm)
        {
            /* returning type description to something that wants it */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype",
                            p_psm->top_exprstate().exs_type()));

            constexpr const char * c_self_name = "exprstate::on_typedescr";

            throw std::runtime_error(tostr(c_self_name,
                                           ": unexpected typedescr for parsing state",
                                           xtag("td", td),
                                           xtag("state", *this)));
        }

        void
        exprstate::on_formal(const rp<Variable> & formal,
                             parserstatemachine * p_psm)
        {
            /* returning type description to something that wants it */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype",
                            p_psm->top_exprstate().exs_type()));

            constexpr const char * c_self_name = "exprstate::on_formal";

            throw std::runtime_error(tostr(c_self_name,
                                           ": unexpected formal-arg for parsing state",
                                           xtag("formal", formal.get()),
                                           xtag("state", *this)));
        }

        void
        exprstate::on_formal_arglist(const std::vector<rp<Variable>> & argl,
                                     parserstatemachine * p_psm)
        {
            /* returning type description to something that wants it */

            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype",
                            p_psm->top_exprstate().exs_type()));

            constexpr const char * c_self_name = "exprstate::on_formal_arglist";

            throw std::runtime_error(tostr(c_self_name,
                                           ": unexpected formal-arg for parsing state",
                                           xtag("argl", argl),
                                           xtag("state", *this)));
        }

        void
        exprstate::on_colon_token(const token_type & tk,
                                  parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_colon";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_comma_token(const token_type & tk,
                                  parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_comma";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_semicolon_token(const token_type & tk,
                                      parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_semicolon";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_singleassign_token(const token_type & tk,
                                         parserstatemachine * /*p_psm*/) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_singleassign_token";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_leftparen_token(const token_type & tk,
                                      parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftparen_token";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_rightparen_token(const token_type & tk,
                                       parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_rightparen";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_leftbrace_token(const token_type & tk,
                                      parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftbrace_token";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_rightbrace_token(const token_type & tk,
                                       parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_rightbrace_token";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_operator_token(const token_type & tk,
                                     parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_operator_token";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_i64_token(const token_type & tk,
                                parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_i64";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_f64_token(const token_type & tk,
                                parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_f64";

            this->illegal_input_error(self_name, tk);
        }

        void
        exprstate::on_input(const token_type & tk,
                            parserstatemachine * p_psm)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));
            log && log(xtag("tk", tk));
            log && log(xtag("state", *this));
            log && log(xtag("psm", *p_psm));
            log && log(xtag("proofoflogging", true));

            switch (tk.tk_type()) {

            case tokentype::tk_def:
                this->on_def_token(tk, p_psm);
                return;

            case tokentype::tk_lambda:
                this->on_lambda_token(tk, p_psm);
                return;

            case tokentype::tk_i64:
                this->on_i64_token(tk, p_psm);
                return;

            case tokentype::tk_f64:
                this->on_f64_token(tk, p_psm);
                return;

            case tokentype::tk_string:
                assert(false);
                return;

            case tokentype::tk_symbol:
                this->on_symbol_token(tk, p_psm);
                return;

            case tokentype::tk_leftparen:
                this->on_leftparen_token(tk, p_psm);
                return;

            case tokentype::tk_rightparen:
                this->on_rightparen_token(tk, p_psm);
                return;

            case tokentype::tk_leftbracket:
            case tokentype::tk_rightbracket:
                assert(false);
                break;

            case tokentype::tk_leftbrace:
                this->on_leftbrace_token(tk, p_psm);
                return;

            case tokentype::tk_rightbrace:
                this->on_rightbrace_token(tk, p_psm);
                return;

            case tokentype::tk_leftangle:
            case tokentype::tk_rightangle:
            case tokentype::tk_dot:
                assert(false);
                return;

            case tokentype::tk_comma:
                this->on_comma_token(tk, p_psm);
                return;

            case tokentype::tk_colon:
                this->on_colon_token(tk, p_psm);
                return;

            case tokentype::tk_doublecolon:
                assert(false);
                return;

            case tokentype::tk_semicolon:
                this->on_semicolon_token(tk, p_psm);
                return;

            case tokentype::tk_singleassign:
                this->on_singleassign_token(tk, p_psm);
                return;

            case tokentype::tk_assign:
            case tokentype::tk_yields:

            case tokentype::tk_plus:
            case tokentype::tk_minus:
            case tokentype::tk_star:
            case tokentype::tk_slash:
                this->on_operator_token(tk, p_psm);
                return;

            case tokentype::tk_type:
            case tokentype::tk_if:
            case tokentype::tk_let:

            case tokentype::tk_in:
            case tokentype::tk_end:
                assert(false);
                return;

            case tokentype::tk_invalid:
            case tokentype::n_tokentype:
                assert(false);
                return;
            }

            assert(false);
        }

        void
        exprstate::on_expr(bp<Expression> expr,
                           parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr));

            assert(false);
        } /*on_expr*/

        void
        exprstate::on_expr_with_semicolon(bp<Expression> expr,
                                          parserstatemachine * /*p_psm*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            const char * c_self_name = "exprstate::on_expr_with_semicolon";

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr));

            throw std::runtime_error
                (tostr(c_self_name,
                       ": unexpected expression for parsing state",
                       xtag("expr", expr),
                       xtag("state", *this)));

            assert(false);
        } /*on_expr_with_semicolon*/

        void
        exprstate::on_symbol(const std::string & symbol_name,
                             parserstatemachine * /*p_psm*/)
        {
            /* unreachable - derived class that can receive
             * will override this method
             */
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("symbol_name", symbol_name));

            assert(false);
        }

        void
        exprstate::print(std::ostream & os) const {
            os << "<exprstate"
               << xtag("this", (void*)this)
               << xtag("type", exs_type_);
            os << ">";
        }

        void
        exprstate::illegal_input_error(const char * self_name,
                                       const token_type & tk) const
        {
            throw std::runtime_error
                (tostr(self_name,
                       ": unexpected input token for parsing state",
                       xtag("token", tk),
                       xtag("state", *this)));
        }

        void
        exprstate::unknown_variable_error(const char * self_name,
                                          const token_type & tk,
                                          parserstatemachine * p_psm) const
        {
            std::string errmsg = tostr("unknown variable name",
                                       xtag("var", tk.text()));

            p_psm->on_error(self_name, std::move(errmsg));
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end exprstate.cpp */
