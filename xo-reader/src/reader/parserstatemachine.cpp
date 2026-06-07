/* file parserstatemachine.cpp
 *
 * author: Roland Conybeare
 */

#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"
#include "pretty_parserstatemachine.hpp"
#include "pretty_envframestack.hpp"
#include "pretty_localenv.hpp"
#include "xo/expression/pretty_expression.hpp"

namespace xo {
    using xo::scm::LocalSymtab;
    using xo::scm::Variable;

    namespace scm {
        bp<Variable>
        parserstatemachine::lookup_var(const std::string & x) const {
            return Variable::from(env_stack_.lookup(x));
        }

        void
        parserstatemachine::upsert_var(bp<Variable> x) {
            env_stack_.upsert(x);
        }

        std::unique_ptr<exprstate>
        parserstatemachine::pop_exprstate() {
            return xs_stack_.pop_exprstate();
        }

        exprstate &
        parserstatemachine::top_exprstate() {
            return xs_stack_.top_exprstate();
        }

        const exprstate &
        parserstatemachine::lookup_exprstate(size_t i) const {
            return *xs_stack_[i];
        }

        void
        parserstatemachine::push_exprstate(std::unique_ptr<exprstate> x) {
            xs_stack_.push_exprstate(std::move(x));
        }

        bp<SymbolTable>
        parserstatemachine::top_envframe() const {
            return env_stack_.top_envframe();
        }

        bp<SymbolTable>
        parserstatemachine::lookup_envframe(std::size_t i) const {
            return env_stack_[i];
        }

        void
        parserstatemachine::push_envframe(const rp<LocalEnv> & x) {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("frame", x));

            env_stack_.push_envframe(x);
        }

        rp<SymbolTable>
        parserstatemachine::pop_envframe() {
            scope log(XO_DEBUG(debug_flag_));

            return env_stack_.pop_envframe();
        }

        void
        parserstatemachine::on_expr(bp<Expression> x)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("x", x),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_expr(x, this);
        }

        void
        parserstatemachine::on_expr_with_semicolon(bp<Expression> x)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("x", x),
                       xtag("psm", this));

            assert(!this->xs_stack_.empty());

            this->xs_stack_.top_exprstate().on_expr_with_semicolon(x, this);
        }

        void
        parserstatemachine::on_symbol(const std::string & x)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("x", x),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_symbol(x, this);
        }

        void
        parserstatemachine::on_semicolon_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_semicolon_token(tk, this);
        }

        void
        parserstatemachine::on_operator_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_operator_token(tk, this);
        }

        void
        parserstatemachine::on_leftbrace_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_leftbrace_token(tk, this);
        }

        void
        parserstatemachine::on_rightbrace_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_rightbrace_token(tk, this);
        }

        void
        parserstatemachine::on_then_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_then_token(tk, this);
        }

        void
        parserstatemachine::on_else_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_else_token(tk, this);
        }

        void
        parserstatemachine::on_f64_token(const token_type & tk)
        {
            scope log(XO_DEBUG(debug_flag_));

            log && log(xtag("tk", tk),
                       xtag("psm", this));

            this->xs_stack_.top_exprstate().on_f64_token(tk, this);
        }

        void
        parserstatemachine::on_error(const char * self_name, std::string errmsg)
        {
            this->result_ = parser_result::error(self_name, std::move(errmsg));
        }

        void
        parserstatemachine::print(std::ostream & os) const {
            os << "<psm";
            os << xtag("stack", &xs_stack_);
            os << xtag("env_stack", &env_stack_);
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.cpp */
