/* @file KalmanFilterInputCallback.hpp */

#pragma once

#include "KalmanFilter.hpp"
#include <xo/reactor/Sink.hpp>
#include <xo/refcnt/Refcounted.hpp>

namespace xo {
    namespace kalman {
        using KalmanFilterInputCallback = reactor::Sink1<rp<KalmanFilterInput>>;
    } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputCallback.hpp */
