/* file parser.cpp
 *
 * author: Roland Conybeare
 */

#include "parser.hpp"
#include "xo/expression/DefineExpr.hpp"
#include "xo/expression/Constant.hpp"
//#include <regex>
#include <stdexcept>

namespace xo {
    using xo::ast::Expression;
    using xo::ast::DefineExpr;
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
            case expractiontype::push1:
                return "push1";
            case expractiontype::push2:
                return "push2";
            case expractiontype::keep:
                return "keep";
            case expractiontype::emit:
                return "emit";
            case expractiontype::pop:
                return "pop";
            case expractiontype::n_expractiontype:
                break;
            }

            return "???";
        }

        expraction
        expraction::keep() {
            return expraction(expractiontype::keep,
                              exprir(),
                              exprstatetype::invalid /*not used*/,
                              exprstatetype::invalid /*not used*/);
        }

        expraction
        expraction::emit(const exprir & ir) {
            return expraction(expractiontype::emit,
                              ir,
                              exprstatetype::invalid /*not used*/,
                              exprstatetype::invalid /*not used*/);
        }

        expraction
        expraction::push2(exprstatetype s1,
                          exprstatetype s2) {
            return expraction(expractiontype::push2,
                              exprir(),
                              s1,
                              s2);
        }

        void
        expraction::print(std::ostream & os) const {
            os << "<expraction";
            os << xtag("type", action_type_);
            os << xtag("ir", expr_ir_);
            os << xtag("push_exs1", push_exs1_);
            os << xtag("push_exs2", push_exs2_);
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

        expraction
        exprstate::on_def() {
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

            /* keyword 'def' introduces a definition:
             *   def pi : f64 = 3.14159265
             *   def sq(x : f64) -> f64 { (x * x) }
             */
            return expraction::push2(exprstatetype::def_0,
                                     /* todo: replace:
                                      *   expect_symbol_or_function_signature()
                                      */
                                     exprstatetype::expect_symbol);
        }

        expraction
        exprstate::on_symbol(const token_type & tk) {
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
                return expraction(expractiontype::pop,
                                  exprir(exprirtype::symbol, tk.text()),
                                  exprstatetype::invalid /*not used*/,
                                  exprstatetype::invalid /*not used*/);

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

                return expraction(expractiontype::pop,
                                  exprir(exprirtype::typedescr, td),
                                  exprstatetype::invalid /*not used*/,
                                  exprstatetype::invalid /*not used*/);
            }

            case exprstatetype::invalid:
            case exprstatetype::n_exprstatetype:
                /* unreachable */
                assert(false);
                return expraction();
            }
        }

        expraction
        exprstate::on_colon() {
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

                return expraction(expractiontype::push1,
                                  exprir(),
                                  exprstatetype::expect_type,
                                  exprstatetype::invalid /*not used*/);
            } else {
                assert(false);
                return expraction();
            }
        }

        expraction
        exprstate::on_singleassign() {
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

                return expraction(expractiontype::push1,
                                  exprir(),
                                  exprstatetype::expect_rhs_expression,
                                  exprstatetype::invalid /*not used*/);
            } else {
                assert(false);
                return expraction();
            }
        }

        expraction
        exprstate::on_f64(const token_type & tk) {
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
                return expraction(expractiontype::pop,
                                  exprir(exprirtype::expression,
                                         Constant<double>::make(tk.f64_value())),
                                  exprstatetype::invalid /*not used*/,
                                  exprstatetype::invalid /*not used*/);
            } else {
                assert(false);
                return expraction();
            }
        }

        expraction
        exprstate::on_input(const token_type & tk) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));
            log && log(xtag("tk", tk));
            log && log(xtag("state", *this));

            switch(tk.tk_type()) {

            case tokentype::tk_def:
                return this->on_def();

            case tokentype::tk_i64:
                assert(false);
                return expraction();

            case tokentype::tk_f64:
                return this->on_f64(tk);

            case tokentype::tk_string:
                assert(false);
                return expraction();

            case tokentype::tk_symbol:
                return this->on_symbol(tk);

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
                return expraction();

            case tokentype::tk_colon:
                return this->on_colon();

            case tokentype::tk_doublecolon:
            case tokentype::tk_semicolon:
                assert(false);
                return expraction();

            case tokentype::tk_singleassign:
                return this->on_singleassign();

            case tokentype::tk_assign:
            case tokentype::tk_yields:

            case tokentype::tk_type:
            case tokentype::tk_lambda:
            case tokentype::tk_if:
            case tokentype::tk_let:

            case tokentype::tk_in:
            case tokentype::tk_end:
                assert(false);
                return expraction();

            case tokentype::tk_invalid:
            case tokentype::n_tokentype:
                assert(false);
                return expraction();
            }

            assert(false);
            return expraction();
        }

        expraction
        exprstate::on_exprir(const exprir & ir) {
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

                if (ir.xir_type() == exprirtype::expression)
                    return expraction::emit(ir);

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
                    /* TODO: do something with def_lhs_type */

                    rp<Expression> rhs_value = ir.expr();
                    rp<Expression> def
                        = DefineExpr::make(this->def_lhs_symbol_,
                                           rhs_value);

                    return expraction(expractiontype::pop,
                                      exprir(exprirtype::expression, def),
                                      exprstatetype::invalid /*not used*/,
                                      exprstatetype::invalid /*not used*/);
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

        // ----- parser -----

        exprstate &
        parser::top_exprstate() {
            std::size_t z = stack_.size();

            if (z == 0) {
                throw std::runtime_error
                    ("parser::top_exprstate: unexpected empty stack");
            }

            return stack_[z-1];
        }

        void
        parser::push_exprstate(const exprstate & exs) {
            std::size_t z = stack_.size();

            stack_.resize(z+1);

            stack_[z] = exs;
        }

        void
        parser::pop_exprstate() {
            std::size_t z = stack_.size();

            if (z > 0)
                stack_.resize(z-1);
        }

        void
        parser::begin_translation_unit() {
            this->push_exprstate
                (exprstate::expect_toplevel_expression_sequence());
        }

        rp<xo::ast::Expression>
        parser::include_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            if (stack_.empty()) {
                throw std::runtime_error(tostr("parser::include_token",
                                                ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack_ is non-empty */
            expraction action = this->top_exprstate().on_input(tk);

            /* loop until reach parsing state that requires more input */
            for (;;) {
                log && log(xtag("action", action));

                switch(action.action_type()) {
                case expractiontype::keep:
                    return nullptr;

                case expractiontype::emit:
                    return action.expr_ir().expr();

                case expractiontype::pop:
                    this->pop_exprstate();

                    if (stack_.empty()) {
                        throw std::runtime_error(tostr("parser::include_token",
                                                       ": pop leaves empty stack"));
                    }

                    action = this->top_exprstate().on_exprir(action.expr_ir());
                    break;

                case expractiontype::push1:
                    this->push_exprstate(action.push_exs1());
                    return nullptr;

                case expractiontype::push2:
                    this->push_exprstate(action.push_exs1());
                    this->push_exprstate(action.push_exs2());
                    return nullptr;

                case expractiontype::invalid:
                case expractiontype::n_expractiontype:
                    /* unreachable */
                    assert(false);
                    return nullptr;
                }
            }
        } /*include_token*/

        void
        parser::print(std::ostream & os) const {
            os << "<parser"
               << xtag("stack", stack_.size())
               << std::endl;

            for (std::size_t i = 0, z = stack_.size(); i < z; ++i) {
                os << "  [" << i << "] "
                   << stack_[i]
                   << std::endl;
            }

            os << ">" << std::endl;
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parser.cpp */
