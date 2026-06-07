/** @file init_procedure2.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-procedure2/ subsystem within ordered iniitalization */
    enum S_procedure2_tag {};

    template <>
    struct InitSubsys<S_procedure2_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_procedure2.hpp */
