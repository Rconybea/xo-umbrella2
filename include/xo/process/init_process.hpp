/* file init_process.hpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    /* tag to represent the process/ subsystem within ordered initialization */
    enum S_process_tag {};

    template<>
    struct InitSubsys<S_process_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_process.hpp */
