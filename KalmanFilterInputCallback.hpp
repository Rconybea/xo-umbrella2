/* @file KalmanFilterInputCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "filter/KalmanFilter.hpp"

namespace xo {
  namespace kalman {
    using KalmanFilterInputCallback = reactor::Sink1<ref::rp<KalmanFilterInput>>;
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputCallback.hpp */
