/** @file init_alloc2.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-alloc2/ subsystem within ordered initialization */
    enum S_alloc2_tag {};

    template <>
    struct InitSubsys<S_alloc2_tag> {
        static void init();
        static InitEvidence require();
    };


} /*namespace xo*/

/* end init_alloc2.hpp */
