/** @file init_indentlog2.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-indentlog2/ subsystem within ordered initialization */
    enum S_indentlog2_tag {};

    template <>
    struct InitSubsys<S_indentlog2_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_indentlog2.hpp */
