/* file init_reactor.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    enum S_reactor_tag {};

    template<>
    struct InitSubsys<S_reactor_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_reactor.hpp */
