/* file init_kalmanfilter.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "xo/subsys/Subsystem.hpp"

namespace xo {
    enum S_kalmanfilter_tag {};

    template<>
    struct InitSubsys<S_kalmanfilter_tag> {
        static void init();
        static InitEvidence require();
    };
} /*namespace xo*/

/* end init_kalmanfilter.hpp */
