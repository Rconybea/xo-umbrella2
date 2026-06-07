/** @file init_stringtable2.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the xo-expression2/ subsystem within ordered initialization */
    enum S_stringtable2_tag {};

    template <>
    struct InitSubsys<S_stringtable2_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_stringtable2.hpp */
