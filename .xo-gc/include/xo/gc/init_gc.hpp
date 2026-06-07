/** @file init_gc.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-gc/ subsystem within ordered initialization */
    enum S_gc_tag {};

    template <>
    struct InitSubsys<S_gc_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_gc.hpp */
