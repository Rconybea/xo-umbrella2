/* GenerationLayout.cpp */

#include "GenerationLayout.hpp"

GenerationLayout::GenerationLayout(const GcGenerationDescription & gendescr,
                                   const ImRect & br,
                                   bool with_labels)
    : gendescr_{gendescr}, bounding_rect_{br}, with_labels_{with_labels}
{
    this->text_dy_ = ImGui::CalcTextSize("SAMPLE TEXT").y;

    if (with_labels_) {
        snprintf(this->rh_text_.data(), rh_text_.capacity(),
                 "%s: %luk",
                 gendescr_.mnemonic_,
                 std::max(gendescr_.gc_threshold_, gendescr_.committed_) / 1024);
        rh_text_.ensure_final_null();

        auto textz = ImGui::CalcTextSize(rh_text_.c_str());

        /* allow margin between rh edge of mem range and beginning of label */
        this->rh_text_dx_ = 5 + textz.x;
    } else {
        this->rh_text_dx_ = 0.0;
    }

    if (with_labels_) {
        this->chart_withlabel_rect_ = bounding_rect_.within_top_margin(text_dy_ + 2);
        this->chart_nolabel_rect_   = chart_withlabel_rect_.within_right_margin(rh_text_dx_);
    } else {
        this->chart_withlabel_rect_ = bounding_rect_;
        this->chart_nolabel_rect_ = bounding_rect_;
    }

    this->mem_rect_from_ = chart_nolabel_rect_.top_fraction(0.45);
    this->mem_rect_to_   = chart_nolabel_rect_.bottom_fraction(0.45);

    if (gendescr_.polarity_ == 1)
        std::swap(this->mem_rect_from_, this->mem_rect_to_);
}

/* GenerationLayout.cpp */
