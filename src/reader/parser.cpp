/* file parser.cpp
 *
 * author: Roland Conybeare
 */

#include "parser.hpp"
#include "define_xs.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/ConvertExpr.hpp"
//#include <regex>
#include <stdexcept>

namespace xo {
    using xo::ast::Expression;
    //using xo::ast::DefineExpr;
    //using xo::ast::ConvertExpr;
    using xo::ast::Constant;
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        const char *
        exprstatetype_descr(exprstatetype x) {
            switch (x) {
            case exprstatetype::invalid:
                return "?invalid";
            case exprstatetype::expect_toplevel_expression_sequence:
                return "expect_toplevel_expression_sequence";
            case exprstatetype::def_0:
                return "def_0";
            case exprstatetype::def_1:
                return "def_1";
            case exprstatetype::def_2:
                return "def_2";
            case exprstatetype::def_3:
                return "def_3";
            case exprstatetype::def_4:
                return "def_4";
            case exprstatetype::def_5:
                return "def_5";
            case exprstatetype::lparen_0:
                return "lparen_0";
            case exprstatetype::lparen_1:
                return "lparen_1";
            case exprstatetype::expect_rhs_expression:
                return "expect_rhs_expression";
            case exprstatetype::expect_symbol:
                return "expect_symbol";
            case exprstatetype::expect_type:
                return "expect_type";
            case exprstatetype::expr_progress:
                return "expr_progress";
            case exprstatetype::n_exprstatetype:
                break;
            }

            return "???";
        }

        bool
        exprstate::admits_definition() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                return true;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* unreachable */
                assert(false);
                return false;
            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
                return false;
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;
            case exprstatetype::expr_progress:
                return false;
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_symbol() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                return false;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
                /* treat symbol as variable name */
                return true;

            case exprstatetype::expect_symbol:
                return true;

            case exprstatetype::expect_type:
                /* treat symbol as typename */
                return true;

            case exprstatetype::expr_progress:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_colon() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
                return false;

            case exprstatetype::def_1:
                return true;

            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with a colon
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_semicolon() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
                return false;
            case exprstatetype::def_5:
                return true;
            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;
            case exprstatetype::expr_progress:
                return true;
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_singleassign() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:

                /*
                 *   def foo       = 1 ;
                 *   def foo : f64 = 1 ;
                 *  ^   ^   ^ ^   ^ ^ ^
                 *  |   |   | |   | | (done)
                 *  |   |   | |   | def_4:expect_rhs_expression
                 *  |   |   | |   def_3
                 *  |   |   | def_2:expect_type
                 *  |   |   def_1
                 *  |   def_0:expect_symbol
                 *  expect_toplevel_expression_sequence
                 *
                 * note that we skip from def_1 -> def_4 if '=' instead of ':'
                 */
            case exprstatetype::def_0:
                return false;

            case exprstatetype::def_1:
                return true;

            case exprstatetype::def_2:
                return false;

            case exprstatetype::def_3:
                return true;

            case exprstatetype::def_4:
            case exprstatetype::def_5:

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with singleassign '='
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_f64() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                return false;

            case exprstatetype::lparen_0:
                return true;

            case exprstatetype::lparen_1:
                return false;

            case exprstatetype::expect_rhs_expression:
                return true;

            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_leftparen() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* input like
                 *   (function(blah...))
                 * not allowed at toplevel;
                 * creates ambiguity e.g. consider
                 *   x := foo
                 *   (bar)
                 *
                 * is rhs 'foo' or 'foo(bar)'
                 */
                return false;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* input like
                 *   def foo : f64 = (
                 *      ^   ^ ^   ^ ^
                 *      |   | |   | def_4
                 *      |   | |   def_3
                 *      |   | def_2
                 *      |   def_1
                 *      def_0
                 *
                 * not allowed or relies on pushing another state
                 */
                return false;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                /* can always begin non-toplevel expression with '(' */
                return true;

            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expect_symbol:
                return false;

            case exprstatetype::expr_progress:
                /* todo: will parse as function call */
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        bool
        exprstate::admits_rightparen() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                return false;

            case exprstatetype::lparen_0:
                /* unreachable -- will have pushed expect_rhs_expression */
                assert(false);
                return false;

            case exprstatetype::lparen_1:
                return true;

            case exprstatetype::expect_rhs_expression:
                return false;

            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expect_symbol:
                return false;

            case exprstatetype::expr_progress:
                /* satisfies expression form */
                return true;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return false;
            }

            return false;
        }

        void
        exprstate::on_def(exprstatestack * p_stack) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_def";

            /* lots of illegal states */
            if (!this->admits_definition())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected keyword 'def' for parsing state",
                                               xtag("state", *this)));
            }

            p_stack->push_exprstate
                (define_xs::def_0(DefineExprAccess::make_empty()));

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
        exprstate::on_symbol(const token_type & tk,
                             exprstatestack * p_stack,
                             rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", p_stack->top_exprstate().exs_type()));

            constexpr const char * self_name = "exprstate::on_symbol";

            if (!this->admits_symbol()) {
                throw std::runtime_error
                    (tostr(self_name,
                           ": unexpected symbol-token for parsing state",
                           xtag("symbol", tk),
                           xtag("state", *this)));
            }

            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                throw std::runtime_error
                    (tostr(self_name,
                           ": unexpected symbol-token at top-level",
                           " (expecting decl|def)",
                           xtag("symbol", tk)));
                break;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* unreachable */
                assert(false);
                return;

            case exprstatetype::lparen_0:
                /* todo: variable reference */
                assert(false);
                break;

            case exprstatetype::lparen_1:
                /* unreachable */

                assert(false);
                break;

            case exprstatetype::expect_rhs_expression:
            {
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

                /* have to do pop first, before sending symbol to
                 * the o.g. symbol-requester
                 */
#ifdef NOT_YET
                p_stack->push_exprstate(exprstate(exprstatetype::expr_progress,
                                                  Variable::make(name, type)));
#endif

#ifdef LATER
                p_stack->pop_exprstate();
                p_stack->top_exprstate().on_symbol(tk.text(),
                                                   p_stack, p_emit_expr);
#endif
                return;
            }

            case exprstatetype::expect_symbol:
            {
                /* have to do pop first, before sending symbol to
                 * the o.g. symbol-requester
                 */
                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();

                p_stack->top_exprstate().on_symbol(tk.text(),
                                                   p_stack, p_emit_expr);
                return;
            }

            case exprstatetype::expect_type: {
                TypeDescr td = nullptr;

                /* TODO: replace with typetable lookup */

                if (tk.text() == "f64")
                    td = Reflect::require<double>();
                else if(tk.text() == "f32")
                    td = Reflect::require<float>();
                else if(tk.text() == "i16")
                    td = Reflect::require<std::int16_t>();
                else if(tk.text() == "i32")
                    td = Reflect::require<std::int32_t>();
                else if(tk.text() == "i64")
                    td = Reflect::require<std::int64_t>();

                if (!td) {
                    throw std::runtime_error
                        (tostr(self_name,
                               ": unknown type name",
                               " (expecting f64|f32|i16|i32|i64)",
                               xtag("typename", tk.text())));
                }

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();
                p_stack->top_exprstate().on_typedescr(td, p_stack, p_emit_expr);
                return;
            }

            case exprstatetype::expr_progress:
                /* illegal input, e.g.
                 *   foo bar
                 */
                assert(false);
                return;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        } /*on_symbol*/

        void
        exprstate::on_typedescr(TypeDescr td,
                                exprstatestack * /*p_stack*/,
                                rp<Expression> * /*p_emit_expr*/)
        {
            /* returning type description to somethign that wants it */

            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            case exprstatetype::def_2:
                this->exs_type_ = exprstatetype::def_3;
                this->cvt_expr_ = ConvertExprAccess::make(td /*dest_type*/,
                                                          nullptr /*source_expr*/);
                this->def_expr_->assign_rhs(this->cvt_expr_);
                //this->def_lhs_td_ = td;

                return;

            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
                assert(false);
                return;

            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_type:
            case exprstatetype::expect_symbol:
                /* unreachable
                 * (this exprstate issues pop instruction from exprstate::on_input()
                 */
                assert(false);
                return;

            case exprstatetype::expr_progress:
                assert(false);
                return;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        exprstate::on_colon(exprstatestack * p_stack) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_colon";

            /* lots of illegal states */
            if (!this->admits_colon())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected colon for parsing state",
                                               xtag("state", *this)));
            }

            if (this->exs_type_ == exprstatetype::def_1) {
                this->exs_type_ = exprstatetype::def_2;

                p_stack->push_exprstate(exprstate::expect_type());
            } else {
                assert(false);
            }
        }

        void
        exprstate::on_semicolon(exprstatestack * p_stack,
                                rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_semicolon";

            if (!this->admits_semicolon())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected semicolon for parsing state",
                                               xtag("state", *this)));
            }

            if (this->exs_type_ == exprstatetype::expr_progress) {
                rp<Expression> expr = this->gen_expr_;

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate(); /* NOT KOSHER. invalidates *this */

                p_stack->top_exprstate().on_expr(expr,
                                                 p_stack,
                                                 p_emit_expr);
                /* control here on input like:
                 *   (1.234;
                 *
                 * a. '(' sets up stack [lparen_0:expect_rhs_expression]
                 *     (see exprstate::on_leftparen())
                 * b. 1.234 pushes (in case operators) [lparen_0:expect_rhs_expression:expr_progress]
                 *     (see exprstate::on_f64())
                 * c. semicolon completes expr_progress [lparen_0:expect_rhs_expression]
                 *     deliver expresssion to expect_rhs_expression.on_expr()
                 *     (see exprstate::on_expr())
                 * d. expr_rhs_expression forwards expression to [lparen_0]
                 * e. lparen_0 advances to [lparen_1]
                 * f. now deliver semicolon;  [lparen_1] rejects
                 */

                p_stack->top_exprstate().on_semicolon(p_stack, p_emit_expr);
            } else if (this->exs_type_ == exprstatetype::def_5) {
                rp<Expression> expr = this->def_expr_;

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate(); /* NOT KOSHER. invalidates *this */

                p_stack->top_exprstate().on_expr(expr,
                                                 p_stack,
                                                 p_emit_expr);
            } else {
                assert(false);
            }
        }

        void
        exprstate::on_singleassign(exprstatestack * p_stack) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_singleassign";

            if (!this->admits_singleassign())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected equals for parsing state",
                                               xtag("state", *this)));
            }

            if ((this->exs_type_ == exprstatetype::def_1)
                || (this->exs_type_ == exprstatetype::def_3))
            {
                this->exs_type_ = exprstatetype::def_4;

                p_stack->push_exprstate(exprstate::expect_rhs_expression());
            } else {
                assert(false);
            }
        }

        void
        exprstate::on_leftparen(exprstatestack * p_stack,
                                rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_leftparen";

            if (!this->admits_leftparen())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected leftparen '(' for parsing state",
                                               xtag("state", *this)));
            }

            if (this->exs_type_ == exprstatetype::expect_rhs_expression) {
                /* push lparen_0 to remember to look for subsequent rightparen. */
                p_stack->push_exprstate(exprstate::lparen_0());
                p_stack->push_exprstate(exprstate::expect_rhs_expression());
            }
        }

        void
        exprstate::on_rightparen(exprstatestack * p_stack,
                                 rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_rightparen";

            if (!this->admits_rightparen())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected rightparen ')' for parsing state",
                                               xtag("state", *this)));
            }

            if (this->exs_type_ == exprstatetype::expr_progress) {
                /* stack may be something like:
                 *
                 *   lparen_0
                 *   expect_rhs_expression
                 *   expr_progress
                 *                   <-- rightparen
                 *
                 * 1. rightparen completes expression-in-progress
                 * 2. rightparen must then match innermost waiting lparen_0
                 */

                /* right paren confirms stack expression */
                rp<Expression> expr = this->gen_expr_;

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate(); /* NOT KOSHER.  invalidates *this */

                if (p_stack->empty()) {
                    throw std::runtime_error(tostr(self_name,
                                                   ": expected non-empty parsing stack"));
                }

                log && log(xtag("stack", p_stack));

                p_stack->top_exprstate().on_expr(expr, p_stack, p_emit_expr);

                /* now deliver rightparen */
                p_stack->top_exprstate().on_rightparen(p_stack, p_emit_expr);
            } else if (this->exs_type_ == exprstatetype::lparen_1) {
                rp<Expression> expr = this->gen_expr_;

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate(); /* NOT KOSHER.  invalidates *this */

                p_stack->top_exprstate().on_expr(expr, p_stack, p_emit_expr);
            }
        }

        void
        exprstate::on_f64(const token_type & tk,
                          exprstatestack * p_stack,
                          rp<Expression> * /*p_emit_expr*/)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_f64";

            if (!this->admits_f64())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected floating-point literal for parsing state",
                                               xtag("state", *this)));
            }

            if (this->exs_type_ == exprstatetype::expect_rhs_expression) {
                /* e.g.
                 *   def pi = 3.14159265;
                 *            \---tk---/
                 */
                p_stack->push_exprstate
                    (exprstate::make_expr_progress
                     (Constant<double>::make(tk.f64_value())));
            } else {
                assert(false);
            }
        }

        void
        exprstate::on_input(const token_type & tk,
                            exprstatestack * p_stack,
                            rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));
            log && log(xtag("tk", tk));
            log && log(xtag("state", *this));

            switch (tk.tk_type()) {

            case tokentype::tk_def:
                this->on_def(p_stack);
                return;

            case tokentype::tk_i64:
                assert(false);
                return;

            case tokentype::tk_f64:
                this->on_f64(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_string:
                assert(false);
                return;

            case tokentype::tk_symbol:
                this->on_symbol(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_leftparen:
                this->on_leftparen(p_stack, p_emit_expr);
                return;

            case tokentype::tk_rightparen:
                this->on_rightparen(p_stack, p_emit_expr);
                return;

            case tokentype::tk_leftbracket:
            case tokentype::tk_rightbracket:
            case tokentype::tk_leftbrace:
            case tokentype::tk_rightbrace:

            case tokentype::tk_leftangle:
            case tokentype::tk_rightangle:
            case tokentype::tk_dot:
            case tokentype::tk_comma:
                assert(false);
                return;

            case tokentype::tk_colon:
                this->on_colon(p_stack);
                return;

            case tokentype::tk_doublecolon:
                assert(false);
                return;

            case tokentype::tk_semicolon:
                this->on_semicolon(p_stack, p_emit_expr);
                return;

            case tokentype::tk_singleassign:
                this->on_singleassign(p_stack);
                return;

            case tokentype::tk_assign:
            case tokentype::tk_yields:

            case tokentype::tk_type:
            case tokentype::tk_lambda:
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
        exprstate::on_expr(ref::brw<Expression> expr,
                           exprstatestack * p_stack,
                           rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", this->exs_type_),
                       xtag("expr", expr));

            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* toplevel expression sequence accepts an
                 * arbitrary number of expressions.
                 *
                 * parser::include_token() returns
                 */

                *p_emit_expr = expr.promote();
                return;
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* unreachable.  see define_xs::on_expr() */
                assert(false);
                return;

            case exprstatetype::lparen_0: {
                this->exs_type_ = exprstatetype::lparen_1; /* wants on_rightparen */
                p_stack->push_exprstate(exprstate::make_expr_progress(expr.promote()));

                return;
            }

            case exprstatetype::lparen_1: {
                this->gen_expr_ = expr.promote();

                /* expect immediate incoming call, this time to on_rightparen() */
                return;
            }

            case exprstatetype::expect_rhs_expression: {

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate(); /* NOT KOSHER.  invalidates *this */

                p_stack->top_exprstate().on_expr(expr,
                                                 p_stack,
                                                 p_emit_expr);

                return;
            }

            case exprstatetype::expect_type:
            case exprstatetype::expect_symbol:
                /* unreachable
                 * (this exprstate issues pop instruction from exprstate::on_input()
                 */
                assert(false);
                return;
            case exprstatetype::expr_progress:
                /* consecutive expressions isn't legal
                 */
                assert(false);
                return;
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        } /*on_expr*/

        void
        exprstate::on_symbol(const std::string & /*symbol_name*/,
                             exprstatestack * /*p_stack*/,
                             rp<Expression> * /*p_emit_expr*/)
        {
            switch(this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* toplevel expression sequence accepts an
                 * arbitrary number of expressions.
                 *
                 * parser::include_token() returns
                 */

                /* NOT IMPLEMENTED */
                assert(false);
                return;
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::def_5:
                /* unreachable */
                assert(false);
                return;

            case exprstatetype::lparen_0:
            case exprstatetype::lparen_1:
                /* NOT IMPLEMENTED */
                assert(false);
                return;

            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_type:
            case exprstatetype::expect_symbol:
                /* unreachable
                 * (this exprstate issues pop instruction from exprstate::on_input()
                 */
                assert(false);
                return;
            case exprstatetype::expr_progress:
                assert(false);
                return;
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return;
            }
        }

        void
        exprstate::print(std::ostream & os) const {
            os << "<exprstate"
               << xtag("type", exs_type_);
            if (def_expr_)
                os << xtag("def_expr", def_expr_);
            if (cvt_expr_)
                os << xtag("cvt_expr", cvt_expr_);
            os << ">";
        }

        // ----- exprstatestack -----

        exprstate &
        exprstatestack::top_exprstate() {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return *(stack_[z-1]);
        }

        void
        exprstatestack::push_exprstate(std::unique_ptr<exprstate> exs) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      xtag("exs", *exs));

            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = std::move(exs);
        }

        std::unique_ptr<exprstate>
        exprstatestack::pop_exprstate() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag),
                      xtag("top.exstype", top_exprstate().exs_type()));

            std::size_t z = stack_.size();

            if (z > 0) {
                std::unique_ptr<exprstate> top = std::move(stack_[z-1]);

                stack_.resize(z-1);

                return top;
            } else {
                return nullptr;
            }
        }

        void
        exprstatestack::print(std::ostream & os) const {
            os << "<exprstatestack"
               << xtag("size", stack_.size())
               << std::endl;

            for (std::size_t i = 0, z = stack_.size(); i < z; ++i) {
                os << "  [" << z-i-1 << "] "
                   << stack_[i]
                   << std::endl;
            }

            os << ">" << std::endl;
        }

        // ----- parser -----

        bool
        parser::has_incomplete_expr() const {
            return !xs_stack_.empty();
        }

        void
        parser::begin_translation_unit() {
            xs_stack_.push_exprstate
                (exprstate::expect_toplevel_expression_sequence());
        }

        rp<Expression>
        parser::include_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag), xtag("tk", tk));

            if (xs_stack_.empty()) {
                throw std::runtime_error(tostr("parser::include_token",
                                                ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack_ is non-empty */

            rp<Expression> retval;

            xs_stack_.top_exprstate().on_input(tk, &xs_stack_, &retval);

            log && log(xtag("retval", retval));

            return retval;
        } /*include_token*/

        void
        parser::print(std::ostream & os) const {
            os << "<parser"
               << std::endl;

            xs_stack_.print(os);

            os << ">" << std::endl;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parser.cpp */
