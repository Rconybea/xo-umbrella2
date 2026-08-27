/* file init_reactor2.hpp
 *
 * author: Roland Conybeare, Aug 2026
 */

#pragma once

#include <xo/subsys/Subsystem.hpp>

namespace xo {
    /* tag to represent the reactor2/ subsystem within ordered initialization */
    enum S_reactor2_tag {};

    template<>
    struct InitSubsys<S_reactor2_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/


/* end init_reactor2.hpp */
