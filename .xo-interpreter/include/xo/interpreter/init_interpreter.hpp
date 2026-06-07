/** @file init_interpreter.hpp
 *
 *  author: Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    /* tag to represent the interpreter/ subsystem in ordered initialization */
    enum S_interpreter_tag {};

    template<>
    struct InitSubsys<S_interpreter_tag> {
        static void init();
        static InitEvidence require();
    };
}

/* end init_interpreter.hpp */
