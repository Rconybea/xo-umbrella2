/* ImRect.hpp */

#pragma once

#include "imgui.h"
#include <algorithm>
#include <utility>

inline ImVec2 operator+(const ImVec2 & p1, const ImVec2 & p2) {
    return ImVec2(p1.x + p2.x, p1.y + p2.y);
}

struct ImRect {
    ImRect() = default;
    ImRect(const ImVec2 & tl, const ImVec2 & br) : top_left_{tl}, bottom_right_{br} {}
    ImRect(float x_lo, float y_lo, float x_hi, float y_hi) : top_left_{x_lo, y_lo}, bottom_right_{x_hi, y_hi} {}

    static ImRect from_xy_span(const ImVec2 & x_span, const ImVec2 & y_span) {
        return ImRect(ImVec2{x_span.x, y_span.x}, ImVec2{x_span.y, y_span.y});
    }

    static void draw_filled_rect_with_label(const char * text,
                                            const char * tooltip,
                                            const ImRect & rect,
                                            ImU32 fillcolor,
                                            ImU32 textcolor,
                                            ImDrawList * draw_list);

    static void draw_filled_rect(const char * tooltip,
                                 const ImRect & rect,
                                 ImU32 fillcolor,
                                 ImDrawList * draw_list);

    std::pair<float, float> x_span() const { return std::make_pair(top_left_.x, bottom_right_.x); }
    std::pair<float, float> y_span() const { return std::make_pair(top_left_.y, bottom_right_.y); }

    const ImVec2 & top_left() const { return top_left_; }
    const ImVec2 & bottom_right() const { return bottom_right_; }

    float x_lo() const { return top_left_.x; }
    float x_hi() const { return bottom_right_.x; }
    float y_lo() const { return top_left_.y; }
    float y_hi() const { return bottom_right_.y; }

    float width() const { return bottom_right_.x - top_left_.x; }
    float height() const { return bottom_right_.y - top_left_.y; }

    float x_mid() const { return 0.5 * (top_left_.x + bottom_right_.x); }
    float y_mid() const { return 0.5 * (top_left_.y + bottom_right_.y); }

    ImVec2 bottom_left() const { return ImVec2(x_lo(), y_hi()); }
    ImVec2 top_right() const { return ImVec2(x_hi(), y_lo()); }

    ImRect with_x_span(float x0, float x1) const {
        return ImRect(ImVec2(x0, top_left_.y), ImVec2(x1, bottom_right_.y));
    }
    ImRect with_y_span(float y0, float y1) const {
        return ImRect(ImVec2(top_left_.x, y0), ImVec2(bottom_right_.x, y1));
    }

    ImRect within_margin(const ImRect & margin) const {
        return ImRect(this->x_lo() + margin.x_lo(),
                      this->y_lo() + margin.y_lo(),
                      this->x_hi() - margin.x_hi(),
                      this->y_hi() - margin.y_hi());
    }

    ImRect within_right_margin(float dx) const {
        return ImRect(this->x_lo(), this->y_lo(), this->x_hi() - dx, this->y_hi());
    }

    ImRect within_top_margin(float dy) const {
        return ImRect(this->x_lo(), this->y_lo() + dy, this->x_hi(), this->y_hi());
    }

    ImRect within_bottom_margin(float dy) const {
        return ImRect(this->x_lo(), this->y_lo(), this->x_hi(), this->y_hi() - dy);
    }

    ImRect translate(const ImVec2 & dist) {
        return ImRect(top_left_ + dist,
                      bottom_right_ + dist);
    }

    /** Require: 0.0 <= p <= 1.0 **/
    ImRect left_fraction(float p, float min_width = 0.0, float max_width = 999999.0) const {
        max_width = std::min(max_width, this->width());

        float w = std::clamp(p * this->width(), min_width, max_width);

        return ImRect(top_left_,
                      ImVec2(this->x_lo() + w, this->y_hi()));
    }

    /** Require: 0.0 <= p <= 1.0 **/
    ImRect right_fraction(float p, float min_width = 0.0, float max_width = 999999.0) const {
        max_width = std::min(max_width, this->width());

        float w = std::clamp(p * this->width(), min_width, max_width);

        return ImRect(ImVec2(this->x_hi() - w, this->y_lo()),
                      bottom_right_);
    }

    /** Require: 0.0 <= p <= q <= 1.0 **/
    ImRect mid_x_fraction(float p, float q) const {
        assert(p <= q);

        float w = this->width();

        return this->with_x_span(this->x_lo() + p * w,
                                 this->x_lo() + q * w);
    }

    /** Require: 0.0 <= p <= 1.0 **/
    ImRect top_fraction(float p, float min_height = 0.0, float max_height = 999999.0) const {
        max_height = std::min(max_height, this->height());

        float h = std::clamp(p * this->height(), min_height, max_height);

        return ImRect(top_left_,
                      ImVec2(this->x_hi(), this->y_lo() + h));
    }

    /** Require: 0.0 <= p <= 1.0 **/
    ImRect bottom_fraction(float p, float min_height = 0.0, float max_height = 999999.0) const {
        max_height = std::min(max_height, this->height());

        float h = std::clamp(p * this->height(), min_height, max_height);

        return ImRect(ImVec2(this->x_lo(), this->y_hi() - h),
                      bottom_right_);
    }

    ImVec2 top_left_{0, 0};
    ImVec2 bottom_right_{0, 0};
};

/* end ImRect.hpp */
