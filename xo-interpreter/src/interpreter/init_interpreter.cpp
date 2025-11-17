/** @file init_interpreter.cpp
 *
 *  author: Roland Conybeare, Nov 2025
 */

#include "init_interpreter.hpp"
#include "StackFrame.hpp"
#include "xo/subsys/Subsystem.hpp"

namespace xo {
    using xo::scm::StackFrame;

    void
    InitSubsys<S_interpreter_tag>::init()
    {
        StackFrame::reflect_self();
    }

    InitEvidence
    InitSubsys<S_interpreter_tag>::require()
    {
        return Subsystem::provide<S_interpreter_tag>("interpreter", &init);
    }

} /*namespace xo*/

/* end init_interpreter.cpp */
