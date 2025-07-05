/* file parserstatemachine.cpp
 *
 * author: Roland Conybeare
 */

#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"

namespace xo {
    using xo::ast::Variable;

    namespace scm {
        rp<Variable>
        parserstatemachine::lookup_var(const std::string & x) const {
            return p_env_stack_->lookup(x);
        }

        void
        parserstatemachine::upsert_var(bp<Variable> x) {
            p_env_stack_->upsert(x);
        }

        std::unique_ptr<exprstate>
        parserstatemachine::pop_exprstate() {
            return p_stack_->pop_exprstate();
        }

        exprstate &
        parserstatemachine::top_exprstate() {
            return p_stack_->top_exprstate();
        }

        void
        parserstatemachine::push_exprstate(std::unique_ptr<exprstate> x) {
            p_stack_->push_exprstate(std::move(x));
        }

        void
        parserstatemachine::push_envframe(envframe x) {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("frame", x));

            p_env_stack_->push_envframe(std::move(x));
        }

        void
        parserstatemachine::pop_envframe() {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            p_env_stack_->pop_envframe();
        }

        void
        parserstatemachine::on_expr(bp<Expression> x)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("x", x),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_expr(x, this);
        }

        void
        parserstatemachine::on_expr_with_semicolon(bp<Expression> x)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("x", x),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_expr_with_semicolon(x, this);
        }

        void
        parserstatemachine::on_symbol(const std::string & x)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("x", x),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_symbol(x, this);
        }

        void
        parserstatemachine::on_semicolon_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("tk", tk),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_semicolon_token(tk, this);
        }

        void
        parserstatemachine::on_operator_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("tk", tk),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_operator_token(tk, this);
        }

        void
        parserstatemachine::on_leftbrace_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("tk", tk),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_leftbrace_token(tk, this);
        }

        void
        parserstatemachine::on_rightbrace_token(const token_type & tk)
        {
            constexpr bool c_debug_flag = true;
            scope log(XO_DEBUG(c_debug_flag));

            log && log(xtag("tk", tk),
                       xtag("psm", *this));

            this->p_stack_
                ->top_exprstate().on_rightbrace_token(tk, this);
        }

        void
        parserstatemachine::print(std::ostream & os) const {
            os << "<psm";
            os << xtag("stack", p_stack_);
            os << xtag("env_stack", p_env_stack_);
            os << xtag("emit_expr", p_emit_expr_);
            os << ">";
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.cpp */
