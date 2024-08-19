/* file parserstatemachine.cpp
 *
 * author: Roland Conybeare
 */

#include "parserstatemachine.hpp"
#include "exprstatestack.hpp"

namespace xo {
    namespace scm {
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
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.cpp */
