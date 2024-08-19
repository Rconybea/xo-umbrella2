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
    } /*namespace scm*/
} /*namespace xo*/


/* end parserstatemachine.cpp */
