/** @file init_type.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-expression2/ subsystem within ordered initialization */
    enum S_type_tag {};

    template <>
    struct InitSubsys<S_type_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_type.hpp */
