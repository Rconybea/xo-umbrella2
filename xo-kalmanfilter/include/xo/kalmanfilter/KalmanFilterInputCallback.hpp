/* @file KalmanFilterInputCallback.hpp */

#pragma once

#include "xo/refcnt/Refcounted.hpp"
#include "KalmanFilter.hpp"

namespace xo {
    namespace kalman {
        using KalmanFilterInputCallback = reactor::Sink1<rp<KalmanFilterInput>>;
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputCallback.hpp */
