/* @file exprstate.cpp */

#include "exprstate.hpp"
#include "define_xs.hpp"
#include "progress_xs.hpp"
#include "paren_xs.hpp"
//#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
//#include "xo/expression/ConvertExpr.hpp"
#include "xo/reflect/Reflect.hpp"

namespace xo {
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
            case exprstatetype::defexpr:
                return "defexpr";
            case exprstatetype::parenexpr:
                return "parenexpr";
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
        exprstate::admits_symbol() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                return false;

            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                /* treat symbol as variable name */
                return true;

            case exprstatetype::expect_symbol:
                return true;

            case exprstatetype::expect_type:
                /* treat symbol as typename */
                return true;

            case exprstatetype::expr_progress:
                /* unreachable */
                assert(false);
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
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable -- redirects to define_xs::admits_colon() etc */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with a colon
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                /* unreachable */
                assert(false);
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
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;
            case exprstatetype::expr_progress:
                /* unreachable */
                assert(false);
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
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs etrc */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with singleassign '='
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                /* unreachable */
                assert(false);
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

            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs */
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
                /* unreachable */
                assert(false);
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
                return false;

            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                return false;

            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expect_symbol:
                return false;

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
        exprstate::admits_f64() const {
            switch (exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs */
                assert(false);
                return false;

            case exprstatetype::expect_rhs_expression:
                return true;

            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::expr_progress:
                /* unreachable */
                assert(false);
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }

            return false;
        }

        void
        exprstate::on_def_token(const token_type & tk,
                                exprstatestack * /*p_stack*/)
        {
            this->illegal_input_error("exprstate::on_def_token", tk);
        }

        void
        exprstate::on_symbol_token(const token_type & tk,
                                   exprstatestack * p_stack,
                                   rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("exstype", p_stack->top_exprstate().exs_type()));

            constexpr const char * c_self_name = "exprstate::on_symbol_token";

            if (!this->admits_symbol()) {
                this->illegal_input_error(c_self_name, tk);
            }

            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs */
                assert(false);
                return;

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
                        (tostr(c_self_name,
                               ": unknown type name",
                               " (expecting f64|f32|i16|i32|i64)",
                               xtag("typename", tk.text())));
                }

                std::unique_ptr<exprstate> self = p_stack->pop_exprstate();
                p_stack->top_exprstate().on_typedescr(td, p_stack, p_emit_expr);
                return;
            }

            case exprstatetype::expr_progress:
                /* unreachable */
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
        exprstate::on_typedescr(TypeDescr /*td*/,
                                exprstatestack * /*p_stack*/,
                                rp<Expression> * /*p_emit_expr*/)
        {
            /* returning type description to something that wants it */

            switch (this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs */
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
                /* unreachable */
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
        exprstate::on_colon_token(const token_type & /*tk*/,
                                  exprstatestack * /*p_stack*/)
        {
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

            assert(false);
        }

        void
        exprstate::on_semicolon_token(const token_type & /*tk*/,
                                      exprstatestack * /*p_stack*/,
                                      rp<Expression> * /*p_emit_expr*/)
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

            assert(false);
        }

        void
        exprstate::on_singleassign_token(const token_type & /*tk*/,
                                         exprstatestack * /*p_stack*/) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_singleassign";

            if (!this->admits_singleassign())
            {
                throw std::runtime_error(tostr(self_name,
                                               ": unexpected equals for parsing state",
                                               xtag("state", *this)));
            }

            assert(false);
        }

        void
        exprstate::on_leftparen_token(const token_type & /*tk*/,
                                      exprstatestack * p_stack,
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
                p_stack->push_exprstate(paren_xs::lparen_0());
                p_stack->push_exprstate(exprstate::expect_rhs_expression());
            }
        }

        void
        exprstate::on_rightparen_token(const token_type & /*tk*/,
                                       exprstatestack * /*p_stack*/,
                                       rp<Expression> * /*p_emit_expr*/)
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

            if (this->exs_type_ == exprstatetype::expr_progress
                || this->exs_type_ == exprstatetype::parenexpr) {
                /* unreachable -- see progress_xs::on_rightparen() etc */
                assert(false);
            }
        }

        void
        exprstate::on_f64_token(const token_type & tk,
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
                    (progress_xs::make
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
                this->on_def_token(tk, p_stack);
                return;

            case tokentype::tk_i64:
                assert(false);
                return;

            case tokentype::tk_f64:
                this->on_f64_token(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_string:
                assert(false);
                return;

            case tokentype::tk_symbol:
                this->on_symbol_token(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_leftparen:
                this->on_leftparen_token(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_rightparen:
                this->on_rightparen_token(tk, p_stack, p_emit_expr);
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
                this->on_colon_token(tk, p_stack);
                return;

            case tokentype::tk_doublecolon:
                assert(false);
                return;

            case tokentype::tk_semicolon:
                this->on_semicolon_token(tk, p_stack, p_emit_expr);
                return;

            case tokentype::tk_singleassign:
                this->on_singleassign_token(tk, p_stack);
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
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable.  redirects to define_xs::on_expr() etc */
                assert(false);
                return;

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
                /* unreachable */
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
            case exprstatetype::defexpr:
            case exprstatetype::parenexpr:
                /* unreachable - redirects to define_xs etc */
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
    } /*namespace scm*/
} /*namespace xo*/

/* end exprstate.cpp */
