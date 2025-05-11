/* @file KalmanFilterOutputCallback.hpp */

#pragma once

#include "xo/reactor/Sink.hpp"
#include "KalmanFilter.hpp"

namespace xo {
    namespace kalman {
        /* callback for consuming kalman filter output */
        using KalmanFilterOutputCallback = reactor::Sink1<rp<KalmanFilterStateExt>>;
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterOutputCallback.hpp */
