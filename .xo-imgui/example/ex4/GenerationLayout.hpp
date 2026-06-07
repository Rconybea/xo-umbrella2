/* GenerationLayout.hpp */

#pragma once

#include "GcGenerationDescription.hpp"
#include "xo/imgui/ImRect.hpp"

/** @class GenerationLayout
 *  @brief layout for displaying a single collector generation.
 *
 *  @text
 *       reserved: xxx committed: xxxx G1: xxx bytes G0: xxx bytes
 *       +--------------------------+--------------------+-------------------------+
 *    to |            G1            |         G0         |                         |
 *       +--------------------------+--------------------+-------------------------+
 *                                                                                   Mem: 28k
 *       +-------------------------------------------------------------------------+
 *  from |                                                                         |
 *       +-------------------------------------------------------------------------+
 *                                                                    ^
 *  layout elements:
 *
 *  <-a-><------------------------------ mem_w ------------------------------------><---b---->
 *       <------------------------- ngc_w ---------------------------->
 *       <--------- G1_w ----------> <------- G0_w ----->
 *
 *  a (lh_text_dx): width for left-hand-side text
 *  b (rh_text_dx): width for right-hand-side text
 *  mem_w:          width for contiguous committed memory
 *  ngc_w:          location (relative to start of GC memory range) of next-collection trigger
 *  G1_w:           width for occupied memory that has survived one GC in this space
 *  G0_w:           width for memory allocated since last GC
 *
 *  @endtext
 *
 **/
struct GenerationLayout {
    GenerationLayout() = default;
    GenerationLayout(const GcGenerationDescription & gendescr, const ImRect & br, bool with_labels);

    const char * name() const { return gendescr_.name_; }
    const char * mnemonic() const { return gendescr_.mnemonic_; }
    const char * gc_type() const { return gendescr_.gc_type_; }
    std::size_t to_G1_size() const { return gendescr_.before_checkpoint_; }
    std::size_t to_G0_size() const { return gendescr_.after_checkpoint_; }
    std::size_t to_gc_threhsold() const { return gendescr_.gc_threshold_; }
    float to_scale() const {
        /** note: deliberate size_t->float conversion here **/
        return gendescr_.scale();
    }
    ImRect to_g1_rect() const {
        return mem_rect_to_.left_fraction(this->to_G1_size() / this->to_scale());
    }
    ImRect to_g0_rect() const {
        return mem_rect_to_.mid_x_fraction(this->to_G1_size() / this->to_scale(),
                                           (this->to_G1_size() + this->to_G0_size()) / this->to_scale());
    }
    ImRect to_alloc_rect() const {
        return mem_rect_to_.left_fraction((this->to_G1_size() + this->to_G0_size()) / this->to_scale());
    }

    ImRect from_alloc_rect() const {
        /* use the same sizing as for source generation */
        return mem_rect_from_.left_fraction((this->to_G1_size() + this->to_G0_size()) / this->to_scale());
    }

    /** size-related statistics for generation to be displayed **/
    GcGenerationDescription gendescr_;

    /** bounding rectangle.  all drawing for generation display will be inside this rectanglge **/
    ImRect bounding_rect_;

    /** true iff text labels enabled **/
    bool with_labels_ = false;

    /** text height in screen units **/
    float text_dy_ = 0.0;

    /** chart rectangle.  bounding rectangle less room for headline text **/
    ImRect chart_withlabel_rect_;
    ImRect chart_nolabel_rect_;

    /** text for RH label.  something like "N: 28k/40k" **/
    xo::flatstring<80> rh_text_;

    /** width of .rh_text in screen units **/
    float rh_text_dx_ = 0.0;

    /** rectangle representing from-space memory range **/
    ImRect mem_rect_from_;
    /** rectangle representing to-space memory range **/
    ImRect mem_rect_to_;
};


/* GenerationLayout.hpp */
