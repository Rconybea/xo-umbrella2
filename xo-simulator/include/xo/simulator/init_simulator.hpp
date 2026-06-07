/* file init_simulator.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    enum S_simulator_tag {};

    template<>
    struct InitSubsys<S_simulator_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_simulator.hpp */
