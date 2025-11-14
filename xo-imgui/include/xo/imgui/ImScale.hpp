/* ImScale.hpp */

#pragma once

#include "imgui.h"
#include "ImSpan.hpp"

/** Linear scale for transforming between two coordinate systems.
 *    f(x) = m.(x - x0) + y0
 *    inv(f)(y) = x0 + (y - y0) / m
 **/
class ImScale {
public:
    ImScale(ImVec2 ref, float m) : ref_{ref}, scale_{m} {}
    /** derive scale f from two reference points @p ref0 and @p ref1.
     *  Promise:
     *  @pre
     *    f(ref0.x) -> ref0.y
     *    f(ref1.x) -> ref1.y
     *  @endpre
     **/
    ImScale(ImVec2 ref0, ImVec2 ref1) : ref_{ref0}, scale_{(ref1.y - ref0.y) / (ref1.x - ref0.x)} {}

    ImVec2 ref() const { return ref_; }
    float scale() const { return scale_; }

    /** evaluate scale f(x) at @p x **/
    constexpr float at(float x) const { return scale_ * (x - ref_.x) + ref_.y; }
    /** evaluate inverse inv(f)(y) at @p y **/
    constexpr float inverse_at(float y) const { return 1.0 / scale_ * (y - ref_.y) + ref_.x; }

    /** construct inverse function inv(f) **/
    ImScale inverse() const { return ImScale(ImVec2(ref_.y, ref_.x), 1.0 / scale_); }

    ImSpan map_span(ImSpan x_span) const { return ImSpan(at(x_span.lo()), at(x_span.hi())); }
    ImVec2 map_span(ImVec2 x_span) const { return ImVec2(at(x_span.x), at(x_span.y)); }
    ImVec2 map_span(float x1, float x2) const { return map_span(ImVec2(x1, x2)); }

    constexpr float operator()(float x) const { return at(x); }

private:
    /** fixed point {x0, f(x0)} **/
    ImVec2 ref_ = ImVec2{0.0, 0.0};
    /** scale.  f(x) - f(x0) = m * (x - x0) **/
    float scale_ = 1.0;
};
