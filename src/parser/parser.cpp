/* file parser.cpp
 *
 * author: Roland Conybeare
 */

#include "parser.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/ConvertExpr.hpp"
//#include <regex>
#include <stdexcept>

namespace xo {
    using xo::ast::Expression;
    using xo::ast::DefineExpr;
    using xo::ast::ConvertExpr;
    using xo::ast::Constant;
    using xo::reflect::Reflect;
    using xo::reflect::TypeDescr;

    namespace scm {
        const char *
        exprirtype_descr(exprirtype x) {
            switch(x) {
            case exprirtype::invalid:
                return "?invalid";
            case exprirtype::empty:
                return "empty";
            case exprirtype::symbol:
                return "symbol";
            case exprirtype::expression:
                return "expression";
            case exprirtype::typedescr:
                return "typedescr";
            case exprirtype::n_exprirtype:
                break;
            }

            return "???exprirtype";
        }

        void
        exprir::print(std::ostream & os) const {
            os << "<exprir"
               << xtag("type", xir_type_)
               << xtag("symbol_name", symbol_name_)
               << xtag("expr", expr_);
            if (td_)
                os << xtag("td", td_->short_name());
            os << ">";
        }

        const char *
        exprstatetype_descr(exprstatetype x) {
            switch(x) {
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
            case exprstatetype::expect_rhs_expression:
                return "expect_rhs_expression";
            case exprstatetype::expect_symbol:
                return "expect_symbol";
            case exprstatetype::expect_type:
                return "expect_type";
            case exprstatetype::n_exprstatetype:
                break;
            }

            return "???";
        }

        const char *
        expractiontype_descr(expractiontype x) {
            switch(x) {
            case expractiontype::invalid:
                return "?invalid";
            case expractiontype::keep:
                return "keep";
            case expractiontype::n_expractiontype:
                break;
            }

            return "???";
        }

        expraction
        expraction::keep() {
            return expraction(expractiontype::keep);
        }

        void
        expraction::print(std::ostream & os) const {
            os << "<expraction";
            os << xtag("type", action_type_);
            os << ">";
        }

        bool
        exprstate::admits_definition() const {
            switch(exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                return true;

            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
                /* note for def_4:
                 * rhs could certainly be a function body that contains
                 * nested defines; but then immediately-enclosing-exprstate
                 * would be a block
                 */
                return false;
            case exprstatetype::expect_rhs_expression:
                return false;
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }
        }

        bool
        exprstate::admits_symbol() const {
            switch(exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
                return false;

            case exprstatetype::expect_rhs_expression:
                /* treat symbol as variable name */
                return true;

            case exprstatetype::expect_symbol:
                return true;

            case exprstatetype::expect_type:
                /* treat symbol as typename */
                return true;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }
        }

        bool
        exprstate::admits_colon() const {
            switch(exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
                return false;

            case exprstatetype::def_1:
                return true;

            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with a colon
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }
        }

        bool
        exprstate::admits_singleassign() const {
            switch(exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
                return false;

            case exprstatetype::def_3:
                return true;

            case exprstatetype::def_4:
            case exprstatetype::expect_rhs_expression:
                /* rhs-expressions (or expressions for that matter)
                 * may not begin with singleassign '='
                 */
            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }
        }

        bool
        exprstate::admits_f64() const {
            switch(exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
            case exprstatetype::def_0:
            case exprstatetype::def_1:
            case exprstatetype::def_2:
            case exprstatetype::def_3:
            case exprstatetype::def_4:
                return false;

            case exprstatetype::expect_rhs_expression:
                return true;

            case exprstatetype::expect_symbol:
            case exprstatetype::expect_type:
                return false;

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                return false;
            }
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

            p_stack->push_exprstate(exprstatetype::def_0);
            /* todo: replace:
             *   expect_symbol_or_function_signature()
             */
            p_stack->push_exprstate(exprstatetype::expect_symbol);

            /* keyword 'def' introduces a definition:
             *   def pi : f64 = 3.14159265
             *   def sq(x : f64) -> f64 { (x * x) }
             */
        }

        expraction
        exprstate::on_symbol(const token_type & tk,
                             exprstatestack * p_stack,
                             rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            constexpr const char * self_name = "exprstate::on_symbol";

            if (!this->admits_symbol()) {
                throw std::runtime_error
                    (tostr(self_name,
                           ": unexpected symbol-token for parsing state",
                           xtag("symbol", tk),
                           xtag("state", *this)));
            }

            switch(this->exs_type_) {
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
                /* unreachable */
                assert(false);
                return expraction();

            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_symbol:
                /* have to do pop first */

                p_stack->pop_exprstate();
                return p_stack->top_exprstate().on_exprir
                    (exprir(exprirtype::symbol, tk.text()), p_stack, p_emit_expr);
                //return expraction::pop(exprir(exprirtype::symbol, tk.text()));

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

                p_stack->pop_exprstate();
                return p_stack->top_exprstate().on_exprir
                    (exprir(exprirtype::typedescr, td), p_stack, p_emit_expr);
                //return expraction::pop(exprir(exprirtype::typedescr, td));
            }

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return expraction();
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

                p_stack->push_exprstate(exprstatetype::expect_type);
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

            if (this->exs_type_ == exprstatetype::def_3) {
                this->exs_type_ = exprstatetype::def_4;

                p_stack->push_exprstate(exprstatetype::expect_rhs_expression);
            } else {
                assert(false);
            }
        }

        void
        exprstate::on_f64(const token_type & tk,
                          exprstatestack * p_stack,
                          rp<Expression> * p_emit_expr)
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
                p_stack->pop_exprstate();

                p_stack->top_exprstate()
                    .on_exprir(exprir(exprirtype::expression,
                                      Constant<double>::make(tk.f64_value())),
                               p_stack,
                               p_emit_expr);
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

            switch(tk.tk_type()) {

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

            case tokentype::tk_rightparen:
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
            case tokentype::tk_semicolon:
                assert(false);
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

        expraction
        exprstate::on_exprir(const exprir & ir,
                             exprstatestack * p_stack,
                             rp<Expression> * p_emit_expr)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));
            log && log(xtag("ir", ir));
            log && log(xtag("state", *this));

            switch(this->exs_type_) {
            case exprstatetype::expect_toplevel_expression_sequence:
                /* toplevel expression sequence accepts an
                 * arbitrary number of expressions.
                 *
                 * parser::include_token() returns
                 */

                if (ir.xir_type() == exprirtype::expression) {
                    *p_emit_expr = ir.expr();
                    return expraction::keep();
                }

                /* NOT IMPLEMENTED */
                assert(false);
                return expraction();
            case exprstatetype::def_0:
                this->exs_type_ = exprstatetype::def_1;
                this->def_lhs_symbol_ = ir.symbol_name();

                return expraction::keep();
            case exprstatetype::def_1:
                /* NOT IMPLEMENTED */
                assert(false);
                return expraction();
            case exprstatetype::def_2:
                this->exs_type_ = exprstatetype::def_3;
                this->def_lhs_td_ = ir.td();

                return expraction::keep();
            case exprstatetype::def_3:
                /* NOT IMPLEMENTED */
                assert(false);
                return expraction();
            case exprstatetype::def_4:
                /* have all the ingredients to create an expression
                 * representing a definition
                 *
                 * 1. if ir_type is a symbol,  interpret as variable name.
                 *    Need to be able to locate variable by type
                 * 2. if ir_type is an expression,  adopt as rhs
                 */
                if (ir.xir_type() == exprirtype::expression) {
                    /* TODO: do something with def_lhs_td */

                    rp<Expression> rhs_value = ir.expr();

                    if (def_lhs_td_)
                        rhs_value = ConvertExpr::make(def_lhs_td_, rhs_value);

                    rp<Expression> def = DefineExpr::make(this->def_lhs_symbol_,
                                                          rhs_value);

                    p_stack->pop_exprstate();
                    return p_stack->top_exprstate()
                        .on_exprir(exprir(exprirtype::expression, def),
                                   p_stack, p_emit_expr);
                } else {
                    assert(false);
                    return expraction();
                }

            case exprstatetype::expect_rhs_expression:
            case exprstatetype::expect_type:
            case exprstatetype::expect_symbol:
                /* unreachable
                 * (this exprstate issues pop instruction from exprstate::on_input()
                 */
                assert(false);
                return expraction();
            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return expraction();
            }
        }

        void
        exprstate::print(std::ostream & os) const {
            os << "<exprstate"
               << xtag("type", exs_type_)
               << xtag("def_lhs_symbol", def_lhs_symbol_);
            if (def_lhs_td_)
                os << xtag("def_lhs_td", def_lhs_td_->short_name());
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

            return stack_[z-1];
        }

        void
        exprstatestack::push_exprstate(const exprstate & exs) {
            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = exs;
        }

        void
        exprstatestack::pop_exprstate() {
            std::size_t z = stack_.size();

            if (z > 0)
                stack_.resize(z-1);
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

        void
        parser::begin_translation_unit() {
            xs_stack_.push_exprstate
                (exprstate::expect_toplevel_expression_sequence());
        }

        rp<Expression>
        parser::include_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            if (xs_stack_.empty()) {
                throw std::runtime_error(tostr("parser::include_token",
                                                ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack_ is non-empty */

            rp<Expression> retval;

            xs_stack_.top_exprstate().on_input(tk, &xs_stack_, &retval);


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
