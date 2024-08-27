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
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.cpp */
