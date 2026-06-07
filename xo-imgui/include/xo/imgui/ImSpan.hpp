/* ImSpan.hpp */

#pragma once

#include "imgui.h"

/* similar to ImVec2, but firmly located in xo territory
 */
struct ImSpan {
    ImSpan() : lo_{0.0}, hi_{0.0} {}
    ImSpan(float lo, float hi) : lo_{lo}, hi_{hi} {}

    float lo() const { return lo_; }
    float hi() const { return hi_; }

    ImSpan normalize() const { return (lo_ <= hi_) ? ImSpan(lo_, hi_) : ImSpan{hi_, lo_}; }
    ImVec2 to_imvec2() const { return ImVec2(lo_, hi_); }

private:
    float lo_ = 0.0;
    float hi_ = 0.0;
};

/* end ImSpan.hpp */
