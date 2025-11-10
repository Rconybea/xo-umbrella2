#ifdef DEBUG
/* xo-imgui/example/ex1/imgui_ex4a.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/Integer.hpp"
#include "xo/object/List.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/alloc/Object.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/object/Integer.hpp"
#include "xo/flatstring/flatstring.hpp"
#include "xo/indentlog/scope.hpp"

#include <SDL.h>
#include <GL/glew.h>
#include <cstddef>

#include "SDL_events.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

//#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>

using xo::gc::generation;

ImVec2 operator+(const ImVec2 & p1, const ImVec2 & p2) {
    return ImVec2(p1.x + p2.x, p1.y + p2.y);
}

struct ImRect {
    ImRect() = default;
    ImRect(const ImVec2 & tl, const ImVec2 & br) : top_left_{tl}, bottom_right_{br} {}
    ImRect(float x_lo, float y_lo, float x_hi, float y_hi) : top_left_{x_lo, y_lo}, bottom_right_{x_hi, y_hi} {}

    static ImRect from_xy_span(const ImVec2 & x_span, const ImVec2 & y_span) {
        return ImRect(ImVec2{x_span.x, y_span.x}, ImVec2{x_span.y, y_span.y});
    }

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

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcGenerationDescription {
    GcGenerationDescription() = default;
    GcGenerationDescription(generation gen,
                            const char * name, const char * mnemonic, const char * gc_type,
                            std::uint8_t polarity,
                            std::size_t tospace_scale,
                            std::size_t before_ckp, std::size_t after_ckp,
                            std::size_t reserved, std::size_t committed, std::size_t gc_threshold)
        : name_{name}, mnemonic_{mnemonic}, gc_type_{gc_type}, polarity_{polarity},
          tospace_scale_{tospace_scale},
          before_checkpoint_{before_ckp}, after_checkpoint_{after_ckp},
          reserved_{reserved}, committed_{committed},
          gc_threshold_{gc_threshold} {}

    /** scale (in bytes) for drawing space **/
    std::size_t scale() const { return std::max(committed_, gc_threshold_); }

    /** nursery or tenured **/
    generation generation_;

    /** "nursery" or "tenured" **/
    const char * name_ = nullptr;

    /** "N" or "T" **/
    const char * mnemonic_ = nullptr;

    /** "incremental" or "full" **/
    const char * gc_type_ = nullptr;

    /** alternates between {0, 1} on each GC **/
    std::uint8_t polarity_ = 0;

    /** size of to-space in bytes represented on screen.
     *  (note however when we animate GC, space roles have already reversed,
     *   so then this will refer to old to-space = new from-space)
     **/
    std::size_t tospace_scale_ = 0;

    std::size_t before_checkpoint_ = 0;
    std::size_t after_checkpoint_ = 0;
    std::size_t reserved_ = 0;
    std::size_t committed_ = 0;
    // G_to_gc_threshold = G1_to_size + gc->config().incr_gc_threshold_;
    std::size_t gc_threshold_ = 0;
};

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcStateDescription {
    using generation = xo::gc::generation;

    GcStateDescription(const GcGenerationDescription & nursery,
                       const GcGenerationDescription & tenured,
                       std::size_t gc_size,
                       std::size_t gc_committed,
                       std::size_t gc_allocated,
                       std::size_t gc_available,
                       std::size_t gc_mlog_size,
                       std::size_t total_promoted,
                       std::size_t total_n_mutation
        );

    const GcGenerationDescription & get_gendescr(generation g) const { return gen_state_v_[gen2int(g)]; }

    std::array<GcGenerationDescription, static_cast<std::size_t>(generation::N)> gen_state_v_;

    /** see @ref GC::size **/
    std::size_t gc_size_ = 0;
    /** see @ref GC::committed **/
    std::size_t gc_committed_ = 0;
    /** see @ref GC::allocated **/
    std::size_t gc_allocated_ = 0;
    /** see @ref GC::available **/
    std::size_t gc_available_ = 0;
    /** see @ref GC::mlog_size **/
    std::size_t gc_mlog_size_ = 0;

    /** see @ref GcStatistics::total_promoted_ **/
    std::size_t total_promoted_ = 0;
    /** see @ref GcStatistics::n_mutation_ **/
    std::size_t total_n_mutation_ = 0;
};

GcStateDescription::GcStateDescription(const GcGenerationDescription & nursery,
                                       const GcGenerationDescription & tenured,
                                       std::size_t gc_size,
                                       std::size_t gc_committed,
                                       std::size_t gc_allocated,
                                       std::size_t gc_available,
                                       std::size_t gc_mlog_size,
                                       std::size_t total_promoted,
                                       std::size_t total_n_mutation)
    : gc_size_{gc_size},
      gc_committed_{gc_committed},
      gc_allocated_{gc_allocated},
      gc_available_{gc_available},
      gc_mlog_size_{gc_mlog_size},
      total_promoted_{total_promoted},
      total_n_mutation_{total_n_mutation}
{
    gen_state_v_[gen2int(generation::nursery)] = nursery;
    gen_state_v_[gen2int(generation::tenured)] = tenured;
}

using xo::gp;
using xo::up;
using xo::Object;
using xo::obj::List;
using xo::obj::Integer;
using xo::rng::xoshiro256ss;
using xo::rng::Seed;

/** details of a single copy event performed by GC **/
struct GcCopyDetail {
    GcCopyDetail(std::size_t z,
                 generation src, std::size_t src_offset, std::size_t src_space_z,
                 generation dest, std::size_t dest_offset, std::size_t dest_z)
        : z_{z},
          src_gen_{src}, src_offset_{src_offset}, src_space_z_{src_space_z},
          dest_gen_{dest}, dest_offset_{dest_offset}, dest_z_{dest_z}
        {}

    /** object size in bytes **/
    std::size_t z_ = 0;
    /** source location **/
    generation src_gen_;
    /** offset from start of allocator **/
    std::size_t src_offset_ = 0;
    /** size of source space.  could store this separately **/
    std::size_t src_space_z_ = 0;

    /** destination location **/
    generation dest_gen_;
    /** offset from start of allocator **/
    std::size_t dest_offset_ = 0;
    /** size of destination space. (could store this separately). **/
    std::size_t dest_z_ = 0;
};

struct AppState {
    using GC = xo::gc::GC;

public:
    AppState();

    std::size_t nursery_tospace_scale() const;
    std::size_t tenured_tospace_scale() const;
    GcStateDescription snapshot_gc_state() const;

    void generate_random_mutation();
    void generate_random_mutations();

public:
    int alloc_per_cycle_ = 1;
    /** if gc triggered, remembers which whether incremental or full **/
    generation upto_ = generation::nursery;
    up<GC> gc_;
    std::size_t next_int_ = 0;
    std::size_t next_root_ = 0;
    std::vector<gp<Object>> gc_root_v_{100};
    Seed<xoshiro256ss> seed_;
    xoshiro256ss rng_{seed_};
    /** remember details for each object copied by GC, so we can animate **/
    std::vector<GcCopyDetail> copy_detail_v_;
    /** max offset for destination, given copied to nursery **/
    std::size_t copy_detail_max_nursery_dest_offset_ = 0;
    std::size_t copy_detail_nursery_dest_size_ = 0;
    std::size_t copy_detail_max_tenured_dest_offset_ = 0;
    std::size_t copy_detail_tenured_dest_size_ = 0;
};

AppState::AppState()
{
    this->gc_ = (GC::make
           (
               {.initial_nursery_z_ = 1024*1024,
                .initial_tenured_z_ = 1024*1024*1024,
                .incr_gc_threshold_ = 16*1024,
                .full_gc_threshold_ = 128*1024,
                .stats_flag_ = true,
                .debug_flag_ = false}));

    Object::mm = gc_.get();

    for (auto & x: gc_root_v_)
        gc_->add_gc_root(x.ptr_address());

    gc_->disable_gc();
}

std::size_t
AppState::nursery_tospace_scale() const {
    std::size_t N1_to_size = gc_->nursery_before_checkpoint();
    std::size_t N_to_committed = gc_->nursery_to_committed();
    std::size_t N_to_incr_gc_threshold = N1_to_size + gc_->config().incr_gc_threshold_;
    std::size_t N_to_scale = std::max(N_to_committed, N_to_incr_gc_threshold);

    return N_to_scale;
}

std::size_t
AppState::tenured_tospace_scale() const {
    std::size_t T1_to_size = gc_->tenured_before_checkpoint();
    std::size_t T_to_committed = gc_->tenured_to_committed();
    std::size_t T_to_full_gc_threshold = T1_to_size + gc_->config().full_gc_threshold_;
    std::size_t T_to_scale = std::max(T_to_committed, T_to_full_gc_threshold);

    return T_to_scale;
}

GcStateDescription
AppState::snapshot_gc_state() const {
    /** NOTE: this gets invoked before GC gets opportunity to run.
     *        in the event that GC does run, from- and to- spaces will
     *        have been reversed (near beginning of GC phase)
     *
     *        This means that nursery_to_reserved() etc. actually refer to from-space
     *        *during gc*
     **/

    // TOOD: may want to use GC::get_gc_statistics() to replace multiple round trips

    return GcStateDescription(GcGenerationDescription
                              (generation::nursery,
                               "nursery",
                               "N",
                               "incremental",
                               gc_->nursery_polarity(),
                               this->nursery_tospace_scale(),
                               gc_->nursery_before_checkpoint(),
                               gc_->nursery_after_checkpoint(),
                               gc_->nursery_to_reserved(),
                               gc_->nursery_to_committed(),
                               gc_->nursery_before_checkpoint() + gc_->config().incr_gc_threshold_),
                              GcGenerationDescription
                              (generation::tenured,
                               "tenured",
                               "T",
                               "full",
                               gc_->tenured_polarity(),
                               this->tenured_tospace_scale(),
                               gc_->tenured_before_checkpoint(),
                               gc_->tenured_after_checkpoint(),
                               gc_->tenured_to_reserved(),
                               gc_->tenured_to_committed(),
                               gc_->tenured_before_checkpoint() + gc_->config().full_gc_threshold_),

                              gc_->size(),
                              gc_->committed(),
                              gc_->allocated(),
                              gc_->available(),
                              gc_->mlog_size(),
                              gc_->native_gc_statistics().total_promoted_,
                              gc_->native_gc_statistics().n_mutation_
        );
}

void
AppState::generate_random_mutation() {
    if (rng_() % 1000 > (5 * 1000) / 7) {
        /* p=16% integer */
        gc_root_v_[next_root_++] = Integer::make(next_int_);
    } else if (rng_() % 1000 > (3 * 1000) / 7) {
        /* p=16% cons */
        gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
        if (random_car.is_null())
            random_car = List::nil;

        /* this will always incorporate existing list as tail of new list */
        gp<List> random_cdr = List::from(gc_root_v_[next_root_]);
        if (random_cdr.is_null())
            random_cdr = List::nil;

        gp<List> random_cons = List::cons(random_car, random_cdr);

        gc_root_v_[next_root_++] = random_cons;
    } else if (rng_() % 1000 > (0 * 1000) / 7) {
        /* p=24% mutation */
        gp<List> random_list = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
        if (!random_list.is_null()) {
            if (rng_() % 2 == 0) {
                /* pick up some random object, assign as head */
                gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
                random_list->assign_head(random_car);
            } else {
                /* pick up some random object; if List, assign tail as tail */
                gp<List> random_cdr = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
                if (!random_cdr.is_null() && !random_cdr->is_nil())
                    random_list->assign_rest(random_cdr->rest());
            }
        }
    }
    if (next_root_ >= gc_root_v_.size())
        this->next_root_ = 0;
}

void
AppState::generate_random_mutations() {
    for (int i = 0; i < this->alloc_per_cycle_; ++i) {
        this->generate_random_mutation();
    }
}

void
draw_filled_rect_with_label(const char * text,
                            const char * tooltip,
                            const ImRect & rect,
                            ImU32 fillcolor,
                            ImU32 textcolor,
                            ImDrawList * draw_list)
{
    draw_list->AddRectFilled(rect.top_left(),
                             rect.bottom_right(),
                             fillcolor);

    if ((rect.width() > 0.0) && (rect.height() > 0.0)) {
        ImGui::SetCursorScreenPos(rect.top_left());
        ImGui::InvisibleButton("ttbutton", ImVec2(rect.width(), rect.height()));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }
    }

    if (text) {
        auto textz = ImGui::CalcTextSize(text);

        /* N1 can be empty: but in that case don't bother to label it */
        if (textz.x < rect.width()) {
            draw_list->AddText(ImVec2(rect.x_mid() - 0.5 * textz.x,
                                      rect.y_mid() - 0.5 * textz.y),
                               textcolor,
                               text);
        }
    }
}

void
draw_filled_rect(const char * tooltip,
                 const ImRect & rect,
                 ImU32 fillcolor,
                 ImDrawList * draw_list)
{
    draw_filled_rect_with_label(nullptr,
                                tooltip,
                                rect,
                                fillcolor,
                                IM_COL32(255, 255, 255, 255),
                                draw_list);
}


using xo::gc::GC;
using xo::gc::GcStatisticsExt;
using xo::gc::GcStatisticsHistory;
using xo::gc::GcStatisticsHistoryItem;
using xo::xtag;
using xo::scope;
using xo::flatstring;
using std::size_t;

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
    flatstring<80> rh_text_;

    /** width of .rh_text in screen units **/
    float rh_text_dx_ = 0.0;

    /** rectangle representing from-space memory range **/
    ImRect mem_rect_from_;
    /** rectangle representing to-space memory range **/
    ImRect mem_rect_to_;
};

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

/**
 *  @p polarity  0 -> draw from-space above to-space; 1 -> draw from-space below to-space
 *  @p p_x1    On exit *p_x1 contains x-coord of right-hand edge of rectangle
 *             depicting potential memory range
 **/
void
draw_generation(const GenerationLayout & layout,
                ImDrawList * draw_list)
{
    //scope log(XO_DEBUG(with_labels));

    using xo::gc::generation;

    /* next GC trigges when G0_to_size reaches this threshold */
    std::size_t G_to_gc_threshold = 0;

    G_to_gc_threshold = layout.gendescr_.gc_threshold_;

    std::size_t G_to_scale = layout.gendescr_.scale();

    /*
     * committed: G_to_committed
     * G1:  G1_to_size
     * G0:  G0_to_size
     * ckp: G1_to_size
     * ngc: G_to_gc_threshold
     *
     *   <----------------------------- committed --------------------------->
     *   <------------------ used ------------------> <-------- free -------->
     *   <------- G1 --------> <-------- G0 -------->
     *  |NNNNNNNNNNNNNNNNNNNNN|nnnnnnnnnnnnnnnnnnnnnn|________________________|
     *                        ^                                      ^
     *                        ckp                                    ngc
     *
     *  in screen coords:
     *
     *  horizontally:
     *
     *         rect.x_lo                                                                   rect.x_hi
     *         v                                                                           v
     *
     *          <--------------------------- display_w -----------------------------> <-+->
     *          <------------------------ ngc_w ---------------------->                 \- rh_text_dx
     *          <------- G1_w ------> <-------- G0_w ------>
     *         ^                     ^                      ^          ^             ^
     *        x0                    G0_x0                  G0_x1     ngc_w          x1
     *     *p_x0                                        *p_g0_x1                 *p_x1
     *
     *  vertically:
     *
     *      <- rect.y_lo
     *     ^
     *
     *     v
     *      <- rect.y_hi
     */

    /* e.g. N1: 34511 bytes */
    char g1_buf[255];

    ImU32 label_color = IM_COL32(255, 255, 192, 255); /*super pale yellow*/

    if (layout.with_labels_) {
        snprintf(g1_buf, sizeof(g1_buf),
                 "reserved: %lu bytes; committed: %lu bytes; %s\u2081: %lu bytes; %s\u2080: %lu bytes",
                 layout.gendescr_.reserved_,
                 layout.gendescr_.committed_,
                 layout.mnemonic(),
                 layout.gendescr_.before_checkpoint_,
                 layout.mnemonic(),
                 layout.gendescr_.after_checkpoint_);

        draw_list->AddText(layout.bounding_rect_.top_left(),
                           label_color,
                           g1_buf);
    }

    if (layout.with_labels_) {
        auto textz = ImGui::CalcTextSize(layout.rh_text_.c_str());

        draw_list->AddText(ImVec2(layout.chart_withlabel_rect_.x_hi() - textz.x,
                                  layout.chart_withlabel_rect_.y_mid() - 0.5 * textz.y),
                           label_color,
                           layout.rh_text_.c_str());
    }

    ImU32 outline_color = IM_COL32(255, 255, 255, 255); /*white*/

    /* chart rectangle */
    draw_list->AddRect(layout.mem_rect_from_.top_left(),
                       layout.mem_rect_from_.bottom_right(),
                       outline_color);

    draw_list->AddRect(layout.mem_rect_to_.top_left(),
                       layout.mem_rect_to_.bottom_right(),
                       outline_color);

    float display_w = layout.mem_rect_from_.width();
    float G1_w = display_w * layout.to_G1_size() / layout.to_scale();
    float G1_x1 = layout.mem_rect_from_.x_lo() + G1_w;
    ImRect G1_rect = layout.to_g1_rect();
    //ImRect G1_rect = layout.mem_rect_to_.left_fraction(layout.to_G1_size() / layout.to_scale());

    /* G1 (i.e. N1 or T1) */
    {
        ImU32   G1_color = IM_COL32(  0, 128,   0, 255);
        ImU32 text_color = IM_COL32(255, 255, 255, 255);

        char buf[255];

        if (layout.with_labels_)
            snprintf(buf, sizeof(buf), "%s\u2081: %luk",
                     layout.mnemonic(), layout.to_G1_size() / 1024); /* N1 / T1 */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2081: %lu - %s survivor size in bytes",
                 layout.mnemonic(),
                 layout.to_G1_size(),
                 layout.name());

        draw_filled_rect_with_label(layout.with_labels_ ? buf : nullptr,
                                    tooltip,
                                    G1_rect,
                                    G1_color,
                                    text_color,
                                    draw_list);
    }

    ImRect G0_rect = layout.to_g0_rect();

    /* G0 (i.e. N0 or T0) */
    {
        ImU32   G0_color = IM_COL32( 32, 192,  32, 255);
        ImU32 text_color = IM_COL32(  0,   0,   0, 255);

        char buf[255];

        if (layout.with_labels_)
            snprintf(buf, sizeof(buf), "%s\u2080: %luk",
                     layout.mnemonic(),
                     layout.to_G0_size() / 1024); /* N(0) */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2080: %lu - %s new alloc size in bytes",
                 layout.mnemonic(), layout.to_G0_size(),
                 layout.name());

        draw_filled_rect_with_label(layout.with_labels_ ? buf : nullptr,
                                    tooltip,
                                    G0_rect,
                                    G0_color,
                                    text_color,
                                    draw_list);
    }

    /* mark where next gc will trigger */
    if (layout.with_labels_) {
        const char * uparrow = reinterpret_cast<const char *>(u8"\u25b3");

        float ngc_w = (display_w * layout.gendescr_.gc_threshold_) / G_to_scale;

        auto tmp = ImGui::CalcTextSize(uparrow);
        std::size_t uparrow_w = tmp.x;
        double ngc_x = layout.chart_withlabel_rect_.x_lo() + ngc_w - uparrow_w/2.0;

        ImVec2 marker_pos(ngc_x, layout.chart_withlabel_rect_.y_hi());

        draw_list->AddText(marker_pos,
                           IM_COL32(255, 128, 128, 255) /*red*/,
                           uparrow);

        ImGui::SetCursorScreenPos(marker_pos);
        ImGui::InvisibleButton("mkbutton", tmp);
        if (ImGui::IsItemHovered()) {
            char marker_tt_buf[255];
            snprintf(marker_tt_buf, sizeof(marker_tt_buf),
                     "Next %s GC when size(%s) >= %lu bytes",
                     layout.gc_type(),
                     layout.name(),
                     layout.gendescr_.gc_threshold_);

            ImGui::SetTooltip("%s", marker_tt_buf);
        }
    }
} /*draw_generation*/

void
draw_nursery(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             GenerationLayout * p_layout)
{
    using xo::gc::generation;

    const GcGenerationDescription & gendescr = gcstate.get_gendescr(generation::nursery);

    GenerationLayout layout(gendescr, rect, with_labels);

    draw_generation(layout, draw_list);

    if (p_layout)
        *p_layout = layout;
}

void
draw_tenured(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             GenerationLayout * p_layout)
{
    using xo::gc::generation;

    const GcGenerationDescription & gendescr = gcstate.get_gendescr(generation::tenured);

    GenerationLayout layout(gendescr, rect, with_labels);

    draw_generation(layout, draw_list);

    if (p_layout)
        *p_layout = layout;
}

/** for history tooltip, choose which statistic to headline **/
enum class gc_history_headline {
    survive,
    promote,
    persist,
    garbage0,
    garbage1,
    garbageN,
    N
};

xo::flatstring<512>
write_gc_history_tooltip(gc_history_headline headline,
                         const GcStatisticsHistoryItem & stats)
{
    xo::flatstring<512> retval;

    xo::flatstring<512> headline_str;
    switch (headline) {
    case gc_history_headline::survive:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "survive: %lu: bytes surviving 1st GC after allocation",
                 stats.survive_z_);
        break;
    case gc_history_headline::promote:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "promote: %lu: bytes surviving 2nd GC; if nursery promote to tenured",
                 stats.promote_z_);
        break;
    case gc_history_headline::persist:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "persist: %lu: bytes surviving 3+ GCs. Only non-zero for full collections",
                 stats.persist_z_);
        break;
    case gc_history_headline::garbage0:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2080: %lu: bytes collected on 1st GC after allocation",
                 stats.garbage0_z_);
        break;
    case gc_history_headline::garbage1:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2081: %lu: bytes collected on 2nd GC after allocation",
                 stats.garbage1_z_);
        break;
    case gc_history_headline::garbageN:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2099: %lu: bytes collected on 3rd or later GC after allocation",
                 stats.garbageN_z_);
        break;
    case gc_history_headline::N:
        assert(false);
        break;
    }



    snprintf(retval.data(), retval.capacity(),
             "%s\n"
             "\n"
             " gcseq: %lu\n"
             " type: %s\n"
             " alloc: %lu\n"
             " survive: %lu\n"
             " promote: %lu\n"
             " persist: %lu\n"
             " garbage\u2080: %lu\n" /*garbage0*/
             " garbage\u2081: %lu\n" /*garbage1*/
             " garbage\u2099: %lu\n" /*garbageN*/
             " effort: %lu dt: %.1lfus\n"
             " copy efficiency: %.1lf%% collection rate: %.0lf bytes/sec",
             headline_str.c_str(),
             stats.gc_seq_,
             (stats.upto_ == generation::nursery) ? "incremental" : "FULL",
             stats.new_alloc_z_,
             stats.survive_z_,
             stats.promote_z_,
             stats.persist_z_,
             stats.garbage0_z_,
             stats.garbage1_z_,
             stats.garbageN_z_,
             stats.effort_z_,
             1e-3 * stats.dt_.scale(),
             100.0 * stats.efficiency(),
             stats.collection_rate()
        );

    return retval.ensure_final_null();
}

/** stacked bar chart
 *
 *  @param gen  if @ref generation::nursery, only display nursery collections.
 *              otherwise display both
 **/
void
draw_gc_history(const GcStateDescription & gcstate,
                generation gen,
                const GcStatisticsHistory & gc_history,
                const ImRect & bounding_rect,
                bool debug_flag,
                ImDrawList * draw_list)
{
    scope log(XO_DEBUG(debug_flag));

    float lm = 10;
    float tm = 25;

    /* we're going to make a bar chart */

    /* x_scale,y_scale in GC units (i.e. bytes) */
    size_t x_scale = gc_history.capacity();
    size_t yplus_scale = 0;
    size_t yminus_scale = 0;

    float display_w = bounding_rect.width()  - lm;
    float display_h = bounding_rect.height() - tm;

    /* 1st loop: figure out max y scale */
    for (const GcStatisticsHistoryItem & stats : gc_history) {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            //size_t  na = stats.new_alloc_z_ - stats.survive_z_; /*new allocs, but dont' double-count survive_z*/
            size_t  sz = stats.survive_z_; /*survive 1st gc */
            size_t  pz = stats.promote_z_; /*survive 2nd gc */
            size_t psz = stats.persist_z_; /*survive 3+ gc */
            size_t g0z = stats.garbage0_z_;
            size_t g1z = stats.garbage1_z_;
            size_t gNz = stats.garbageN_z_;

            if (yplus_scale < sz + pz + psz)
                yplus_scale = sz + pz + psz;

            if (yminus_scale < g0z + g1z + gNz)
                yminus_scale = g0z + g1z + gNz;
        } else {
            ;
        }
    }

    /* y-coord of x-axis */
    float y_zero = bounding_rect.y_lo() + tm + (display_h * yplus_scale) / (yplus_scale + yminus_scale);

    float y_scale = yplus_scale + yminus_scale;

    /* width of 1 bar in screen coords */
    constexpr float c_min_bar_w = 5.0;
    float bar_w = std::max(c_min_bar_w, display_w / gc_history.capacity());

    /* 2nd loop: draw bars */
    std::size_t i = 0;
    for (const GcStatisticsHistoryItem & stats : gc_history)
    {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            /*
             *     ys_lo   +--+
             *             |  |   survive_z  (survived 1st GC)
             *             |  |
             *     yp_lo   +--+
             *             |  |   promote_z  (sruvived 2nd GC)
             *             |  |
             *   ypsz_lo   +--+
             *             |  |   persist_z  (survived 3+ GCs)
             *             |  |
             *    y_zero   +--+
             *             |  |   gN  (killed on 3+ GC)
             *             |  |
             *    ygN_hi   +--+
             *             |  |   g1  (killed on 2nd GC)
             *             |  |
             *    yg1_hi   +--+
             *             |  |   g0  (killed on 1st GC)
             *             |  |
             *    yg0_hi   +--+
             */

            ImU32  persist_color = IM_COL32(  0,  64, 192, 255); /*darker blue*/
            ImU32  promote_color = IM_COL32(  0, 128,   0, 255); /*darker green*/
            ImU32  survive_color = IM_COL32( 32, 192,  32, 255); /*lighter green*/
            ImU32 garbageN_color = IM_COL32(255, 128,  64, 255); /*darker orange*/
            ImU32 garbage1_color = IM_COL32(255, 192, 128, 255); /*medium orange*/
            ImU32 garbage0_color = IM_COL32(255, 255, 192, 255); /*pale yellow*/

            /* x-coordinates of bar */
            float x_lo = bounding_rect.x_lo() + lm + i * bar_w;
            float x_hi = x_lo + bar_w - 1;
            ImVec2 x_span{x_lo, x_hi};

            /* y-coordinates of persist bar (survived 3+ GCs) */
            float ypsz_lo = (y_zero
                             - (display_h * stats.persist_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::persist, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ypsz_lo, y_zero)),
                                 persist_color,
                                 draw_list);
            }
            /* y-coordinates of promote bar (survived 2nd GC) */
            float yp_hi = ypsz_lo;
            float yp_lo = (yp_hi
                           - (display_h * stats.promote_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::promote, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(yp_lo, yp_hi)),
                                 promote_color,
                                 draw_list);
            }

            /* y-coordinates of survivor bar (survived 1st GC) */
            float ys_hi = yp_lo;
            float ys_lo = (ys_hi - (display_h * stats.survive_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::survive, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ys_lo, ys_hi)),
                                 survive_color,
                                 draw_list);
            }

            // -----------------------------------------------------------

            /* y-coordinates of garbageN bar (killed on 3+ GC) */
            float ygN_lo = y_zero;
            float ygN_hi = (y_zero
                            + (display_h * stats.garbageN_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbageN, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ygN_lo, ygN_hi)),
                                 garbageN_color,
                                 draw_list);
            }

            /* y-coordinates of garbage1 bar (killed on 2nd GC) */
            float yg1_lo = ygN_hi;
            float yg1_hi = (yg1_lo
                            + (display_h * stats.garbage1_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbage1, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect(ImVec2(x_lo, yg1_lo), ImVec2(x_hi, yg1_hi)),
                                 garbage1_color,
                                 draw_list);
            }

            /* y-coordinates of garbage0 bar (killed on 1st GC) */
            float yg0_lo = yg1_hi;
            float yg0_hi = (yg0_lo
                            + (display_h * stats.garbage0_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbage0, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect(ImVec2(x_lo, yg0_lo), ImVec2(x_hi, yg0_hi)),
                                 garbage0_color,
                                 draw_list);
            }
        } else {
            /* draw nothing */
            ;
        }

        ++i;
    }

    log && log(xtag("i", i));
}

void
draw_gc_efficiency(const GcStateDescription & gcstate,
                   //generation gen,
                   const GcStatisticsHistory & gc_history,
                   const ImRect & bounding_rect,
                   bool debug_flag,
                   ImDrawList * draw_list)
{
    scope log(XO_DEBUG(debug_flag));

    float lm = 10;
    float tm = 25;

    /* we're going to make a level chart */

    /* x_scale,y_scale in GC units (i.e. bytes) */
    size_t x_scale = gc_history.capacity();
    size_t yplus_scale = 1;
    size_t yminus_scale = 0;

    float display_w = bounding_rect.width()  - lm;
    float display_h = bounding_rect.height() - tm;

#ifdef NOPE // don't need this.  y-scale is [0.0, 1.0]
    /* 1st loop: figure out max y scale */
    for (const GcStatisticsHistoryItem & stats : gc_history) {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            //size_t  na = stats.new_alloc_z_ - stats.survive_z_; /*new allocs, but dont' double-count survive_z*/
            size_t  sz = stats.survive_z_; /*survive 1st gc */
            size_t  pz = stats.promote_z_; /*survive 2nd gc */
            size_t psz = stats.persist_z_; /*survive 3+ gc */
            size_t g0z = stats.garbage0_z_;
            size_t g1z = stats.garbage1_z_;
            size_t gNz = stats.garbageN_z_;

            if (yplus_scale < sz + pz + psz)
                yplus_scale = sz + pz + psz;

            if (yminus_scale < g0z + g1z + gNz)
                yminus_scale = g0z + g1z + gNz;
        } else {
            ;
        }
    }
#endif

    /* y-coord of x-axis */
    float y_zero = bounding_rect.y_lo() + tm + display_h;
    //float y_zero = bounding_rect.y_lo() + tm + (display_h * yplus_scale) / (yplus_scale + yminus_scale);

    float y_scale = 1.0;

    /* width of 1 bar in screen coords */
    constexpr float c_min_bar_w = 5.0;
    float bar_w = std::max(c_min_bar_w, display_w / gc_history.capacity());

    /* TODO: use temporary arena */
    std::vector<ImVec2> line_points;
    line_points.reserve(gc_history.size());

    ImU32 average_color = IM_COL32(255, 255,  64, 255); /*solid yellow*/
    ImU32  sample_color = IM_COL32(255, 255, 255, 255); /*white*/

    /* 2nd loop: draw levels */
    std::size_t i = 0;
    for (const GcStatisticsHistoryItem & stats : gc_history)
    {
        //std::vector<ImVec2> line_points = { /* your points */ };
        //draw_list->AddPolyline(line_points.data(), line_points.size(),
        //                       IM_COL32(255, 255, 0, 255), false, 2.0f);

        float y = y_zero - display_h * stats.efficiency();
        float y_mean = y_zero - display_h * stats.average_efficiency();

        /* x-coordinates of point */
        float x = bounding_rect.x_lo() + lm + i * bar_w + 0.5 * bar_w;

        line_points.push_back(ImVec2(x, y_mean));

        draw_list->AddCircleFilled(ImVec2(x, y), 2.0f, sample_color);

        ++i;
    }

    draw_list->AddPolyline(line_points.data(),
                           line_points.size(),
                           average_color,
                           false,
                           1.0f /*line width?*/);

    log && log(xtag("i", i));
} /*draw_gc_efficiency*/


void
draw_gc_alloc_state(const GcStateDescription & gcstate,
                    const ImRect & canvas_rect,
                    ImDrawList * draw_list,
                    GenerationLayout * p_nursery_layout,
                    GenerationLayout * p_tenured_layout
                    //ImRect * p_nursery_alloc_rect,
                    //ImRect * p_tenured_alloc_rect
    )
{
    constexpr float c_est_chart_text_height = 14.0;
    constexpr float c_min_h =  7;  // chart bar height
    constexpr float c_max_h = 40;  // chart bar height

    /* bounding rectange for nursery display */
    ImRect N_space_rect = canvas_rect.top_fraction(0.5,
                                                   c_min_h + c_est_chart_text_height,
                                                   c_max_h + c_est_chart_text_height);

    //assert(N_space_rect.height() >= c_min_h + c_est_chart_text_height);
    //assert(N_space_rect.height() <= c_max_h + c_est_chart_text_height);

    /* rectangle representing allocated nursery range */
    //float N_x1 = 0.0;
    GenerationLayout N_layout;

    draw_nursery(gcstate,
                 true /*with_labels*/,
                 N_space_rect,
                 draw_list,
                 &N_layout);

    float N_x1 = N_layout.chart_nolabel_rect_.x_hi();

    if (p_nursery_layout)
        *p_nursery_layout = N_layout;

//    if (p_nursery_alloc_rect)
//        *p_nursery_alloc_rect = N_layout.to_alloc_rect();

    /* N0_to_size..N_to_scale: in bytes */
    std::size_t N_to_scale = gcstate.gen_state_v_[gen2int(generation::nursery)].tospace_scale_;

    /* display_w .. N0_h : viewportcoords */
    std::size_t display_w = canvas_rect.width();

    std::size_t x0 = canvas_rect.x_lo();
    std::size_t x1 = canvas_rect.x_hi();

    // now turn to Tenured space

    std::size_t T_to_scale = gcstate.gen_state_v_[gen2int(generation::tenured)].tospace_scale_;

    /* want to put to-scale image of nursery next to to-scale image of tenured;
     * but also want space between them.
     */
    float TplusN_to_scale = N_to_scale + T_to_scale;
    /* space between T, N images */
    float TplusN_spacer = 10;

    /* bounding rectange for tenured display */
    ImRect T_space_rect = (canvas_rect
                           .within_bottom_margin(c_est_chart_text_height)
                           .bottom_fraction(0.5,
                                            c_min_h + c_est_chart_text_height,
                                            c_max_h + c_est_chart_text_height));

    if (N_space_rect.y_hi() > T_space_rect.y_lo()) {
        T_space_rect = T_space_rect.translate(ImVec2(0, N_space_rect.y_hi() - T_space_rect.y_lo()));
    }

    assert(T_space_rect.y_lo() >= N_space_rect.y_hi());

    /* for smaller image of nursery */
    //std::size_t t_y0 = canvas_rect.y_lo() + 70 + alloc_height + 20;

    /* for side-by-side tenured + nursery, with both on same scale
     * 2nd term is horiz space used for N label like 'Mem: 28k'
     */
    std::size_t adj_display_w = display_w - (N_space_rect.x_hi() - N_x1);

    /* bounding rectangle for secondary nursery display */
    ImRect np_rect(ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)),
                          T_space_rect.y_lo() + c_est_chart_text_height),
                   ImVec2(x0 + adj_display_w,
                          T_space_rect.y_hi()));

    // redraw nursery to same scale as tenured
    {
        draw_list->AddLine(N_space_rect.bottom_left(), np_rect.top_left(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);
        draw_list->AddLine(ImVec2(N_x1, N_space_rect.y_hi()), np_rect.top_right(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);

        draw_nursery(gcstate,
                     false /*no labels*/,
                     np_rect,
                     draw_list,
                     nullptr);
    }

    /* rectangle representing allocated tenured range */
    GenerationLayout T_layout;

    draw_tenured(gcstate,
                 true /*with labels*/,
                 ImRect(ImVec2(x0, T_space_rect.y_lo()),
                        ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)) - TplusN_spacer,
                               T_space_rect.y_hi())),
                 draw_list,
                 &T_layout);

    if (p_tenured_layout)
        *p_tenured_layout = T_layout;

//    if (p_tenured_alloc_rect)
//        *p_tenured_alloc_rect = T_layout.to_alloc_rect();

} /*draw_gc_alloc_state*/

void
draw_gc_state(const AppState & app_state,
              const GcStateDescription & gcstate,
              const ImRect & canvas_rect,
              ImDrawList * draw_list,
              GenerationLayout * p_nursery_layout,
              GenerationLayout * p_tenured_layout,
              //ImRect * p_nursery_alloc_rect,
              //ImRect * p_tenured_alloc_rect,
              ImRect * p_history_rect)
{
    // draw stuff
    draw_list->AddRect(canvas_rect.top_left(),
                       canvas_rect.bottom_right(),
                       IM_COL32(255, 255, 255, 255));

    /* TODO: does this reset coord space? */
    ImRect alloc_rect;
    {
        ImGui::BeginChild("top pane", ImVec2(0, 105), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);

        alloc_rect = ImRect(canvas_rect.top_left() + ImGui::GetWindowContentRegionMin(),
                            canvas_rect.top_left() + ImGui::GetWindowContentRegionMax());
        ImRect draw_rect = alloc_rect.within_margin(ImRect(50, 10, 70, 10));

        draw_list->PushClipRect(draw_rect.top_left(), draw_rect.bottom_right());

        draw_gc_alloc_state(gcstate,
                            draw_rect,
                            draw_list,
                            p_nursery_layout,
                            p_tenured_layout
                            //p_nursery_alloc_rect,
                            //p_tenured_alloc_rect
            );

        draw_list->PopClipRect();

        ImGui::EndChild();
    }

    ImRect history_rect;
    {
        ImGui::BeginChild("left pane", ImVec2(800, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

        /* history below alloc area */
        history_rect = ImRect(alloc_rect.bottom_left() + ImGui::GetWindowContentRegionMin(),
                              alloc_rect.bottom_left() + ImGui::GetWindowContentRegionMax());

         if (p_history_rect)
             *p_history_rect = history_rect;

         draw_list->PushClipRect(history_rect.top_left(), history_rect.bottom_right());

         float lm = 50;
         float rm = 70;
         float tm = 10;
         std::size_t x0 = history_rect.x_lo() + lm;
         std::size_t x1 = history_rect.x_hi() - rm;
         std::size_t h_y0 = history_rect.y_lo() + tm;

         /* just incremental (nursery) collections */
         ImRect incremental_rect = history_rect.top_fraction(0.33);

         draw_gc_history(gcstate,
                         generation::nursery,
                         app_state.gc_->gc_history(),
                         incremental_rect,
                         false /*debug_flag*/,
                         draw_list);

         /* just full (nursery+tenured) collections */
         ImRect full_rect = history_rect.bottom_fraction(0.67).top_fraction(0.5);

         /* both nursery + full collections */
         draw_gc_history(gcstate,
                         generation::tenured,
                         app_state.gc_->gc_history(),
                         full_rect,
                         false /*debug_flag*/,
                         draw_list);

         ImRect efficiency_rect = history_rect.bottom_fraction(0.67).bottom_fraction(0.5);

         draw_gc_efficiency(gcstate,
                            app_state.gc_->gc_history(),
                            efficiency_rect,
                            false /*debug_flag*/,
                            draw_list);

         draw_list->PopClipRect();

         ImGui::EndChild();
    }

    ImGui::Text("placeholder text");

    /* BeginChild() again ? */

#ifdef NOPE
    draw_list->AddCircleFilled(ImVec2(canvas_p0.x + 50, canvas_p0.y + 50),
                               30.0f, IM_COL32(255, 0, 0, 255));

    draw_list->AddText(ImVec2(canvas_p0.x + 10, canvas_p0.y + 10),
                       IM_COL32(255, 255, 255, 255), "Hello 2D!");
#endif
}

struct DrawState;

struct AnimateGcCopyCb : public xo::gc::GcCopyCallback {
    using generation = xo::gc::generation;

    explicit AnimateGcCopyCb(AppState * appstate, DrawState * drawstate)
        : p_app_state_{appstate}, p_draw_state_{drawstate} {}

    virtual void notify_gc_copy(std::size_t z,
                                const void * src_addr, const void * dest_addr,
                                generation src_gen, generation dest_gen);

    AppState * p_app_state_ = nullptr;
    DrawState * p_draw_state_ = nullptr;
};

enum class draw_state_type {
    alloc,
    animate_gc
};

struct DrawState {
    up<xo::gc::GcCopyCallback> make_gc_copy_animation(AppState * app_state) {
        return std::make_unique<AnimateGcCopyCb>(app_state, this);
    }

    draw_state_type state_type_ = draw_state_type::alloc;

    /** budgeted time period over which to animate gc copy **/
    int animate_copy_budget_ms_ = 2000;
    /** start time of current copy animation **/
    std::chrono::steady_clock::time_point animate_copy_t0_;
    /** when animating copy step, display objects from AppState::copy_detail_v_[i]
     *  where i < .animate_copy_hi_ / 100 * AppState::copy_detail_v_.size()
     **/
    float animate_copy_hi_pct_ = 0;

    ImDrawList * gcw_draw_list_ = nullptr;

    /** draw area **/
    ImVec2 gcw_canvas_p0_;
    ImVec2 gcw_canvas_p1_;

    /** layout for nursery display **/
    GenerationLayout gcw_nursery_layout_;
    /** layout for tenured display **/
    GenerationLayout gcw_tenured_layout_;

    /** rect displaying gc history (strip charts) **/
    ImRect gcw_history_rect_;
};

ImRect map_src_alloc_to_screen(const GcCopyDetail & copy_detail,
                               const ImRect & space_rect)
{
    // TODO: methods on copy_detail / and/or ImPoint

    auto [x_coord_lo, x_coord_hi] = space_rect.x_span();

    double w0 = copy_detail.src_offset_ / static_cast<double>(copy_detail.src_space_z_);
    float src0_x = ((1.0 - w0) * x_coord_lo) + (w0 * x_coord_hi);

    double w1 = ((copy_detail.src_offset_ + copy_detail.z_)
                 / static_cast<double>(copy_detail.src_space_z_));
    float src1_x = ((1.0 - w1) * x_coord_lo) + (w1 * x_coord_hi);

    return space_rect.with_x_span(src0_x, src1_x);
}

ImRect map_dest_alloc_to_screen(const GcCopyDetail & copy_detail,
                                const ImRect & space_rect)
{
    /* for from-space, want to use full width of memory space */

    auto [x_coord_lo, x_coord_hi] = space_rect.x_span();

    // dest_space_z_ ?

    double w0 = copy_detail.dest_offset_ / static_cast<double>(copy_detail.dest_z_);
    float dest0_x = ((1.0 - w0) * x_coord_lo) + (w0 * x_coord_hi);

    double w1 = ((copy_detail.dest_offset_ + copy_detail.z_)
                 / static_cast<double>(copy_detail.dest_z_));
    float dest1_x = ((1.0 - w1) * x_coord_lo) + (w1 * x_coord_hi);

    return space_rect.with_x_span(dest0_x, dest1_x);
}

/* editor bait: animate_copy() */
void animate_gc_copy(const AppState & app_state,
                     const DrawState & draw_state,
                     ImDrawList * draw_list)
{
    /* NOTE: this only runs during GC copy.
     *       draw_state.gcw_nursery_layout_ and gcw_tenured_layout_
     *       are taken from snapshots made before GC began,
     *       ergo before to/from spaces were swapped
     */
    ImRect nursery_src_rect = draw_state.gcw_nursery_layout_.to_alloc_rect();
    ImRect tenured_src_rect = draw_state.gcw_tenured_layout_.to_alloc_rect();

    ImRect nursery_dest_rect = draw_state.gcw_nursery_layout_.mem_rect_from_;
    ImRect tenured_dest_rect;

    if (app_state.upto_ == generation::nursery) {
        tenured_dest_rect = draw_state.gcw_tenured_layout_.mem_rect_to_;
    } else {
        tenured_dest_rect = draw_state.gcw_tenured_layout_.mem_rect_from_;
    }

    std::size_t n_copy = app_state.copy_detail_v_.size();
    /* grade from black-to-white between lo_copy and hi_copy.
     * Note we allow animate_copy_hi_pct_ > 100.0
     */
    float lo_copy = 0.01 * std::max(0.0, draw_state.animate_copy_hi_pct_ - 14.0) * n_copy;
    float hi_copy = 0.01 * draw_state.animate_copy_hi_pct_ * n_copy;

    /* remember max copy offset seen in {nursery, tenured} space respectively,
     * so we can label it
     */
    std::size_t last_nursery_dest_offset = 0;
    ImRect last_nursery_dest_rect;
    std::size_t first_tenured_dest_offset = 0;
    ImRect first_tenured_dest_rect;
    std::size_t last_tenured_dest_offset = 0;
    ImRect last_tenured_dest_rect;

    std::size_t i_copy = 0;
    for (const auto & copy_detail : app_state.copy_detail_v_) {
        /* cutout for each copied object */
        {
            float wt = (i_copy > lo_copy) ? static_cast<float>(i_copy) / hi_copy : 0.0;
            /* grey fading to black */
            //ImU32 color = IM_COL32(wt*128, 64+wt*64, wt*128, 255);
            ImU32 color = IM_COL32( 96, 224, 255, 255);

            ImRect src_rect;

            if (copy_detail.src_gen_ == generation::nursery) {
                src_rect = map_src_alloc_to_screen(copy_detail, nursery_src_rect);
            } else {
                src_rect = map_src_alloc_to_screen(copy_detail, tenured_src_rect);
            }

            draw_list->AddRectFilled(src_rect.top_left(),
                                     src_rect.bottom_right(),
                                     color);
        }

        if (copy_detail.dest_gen_ == generation::nursery) {
            last_nursery_dest_rect = map_dest_alloc_to_screen(copy_detail, nursery_dest_rect);
            last_nursery_dest_offset = copy_detail.dest_offset_;
        } else if (copy_detail.dest_gen_ == generation::tenured) {
            last_tenured_dest_rect = map_dest_alloc_to_screen(copy_detail, tenured_dest_rect);
            last_tenured_dest_offset = copy_detail.dest_offset_;

            if (first_tenured_dest_rect.width() == 0) {
                first_tenured_dest_rect = last_tenured_dest_rect;
                first_tenured_dest_offset = copy_detail.dest_offset_;
            }
        }

        if (++i_copy >= hi_copy) {
            break;
        }
    }

    if (last_nursery_dest_rect.width() > 0.0) {
        //ImU32 color = IM_COL32(64, 255, static_cast<int>(64 + (128 * wt)), 255);
        ImU32 color = IM_COL32(  0, 96, 192, 255);

        draw_list->AddRectFilled(nursery_dest_rect.top_left(),
                                 last_nursery_dest_rect.bottom_right(),
                                 color);

        char buf[255];
        snprintf(buf, sizeof(buf), "N\u2081: %luk", last_nursery_dest_offset / 1024);

        auto textz = ImGui::CalcTextSize(buf);

        ImU32  text_color = IM_COL32(255, 255, 255, 255); /*black*/
        ImVec2 text_pos   = ImVec2(0.5 * (nursery_dest_rect.x_lo()
                                          + last_nursery_dest_rect.x_hi()
                                          - textz.x),
                                   nursery_dest_rect.y_mid() - 0.5 * textz.y);

        if (text_pos.x < nursery_dest_rect.x_lo())
            text_pos.x = nursery_dest_rect.x_lo() + 2;
        else if (text_pos.x < nursery_dest_rect.x_lo() + 0.5 * textz.x)
            text_pos.x = nursery_dest_rect.x_lo() + 0.5 * textz.x;

        draw_list->AddText(text_pos, text_color, buf);

    }

    if (last_tenured_dest_rect.width() > 0.0) {
        ImU32 color = IM_COL32(  0,  96, 192, 255);

        draw_list->AddRectFilled(first_tenured_dest_rect.top_left(),
                                 last_tenured_dest_rect.bottom_right(),
                                 color);

        char buf[255];
        snprintf(buf, sizeof(buf), "+%luk", (last_tenured_dest_offset - first_tenured_dest_offset) / 1024);

        auto textz = ImGui::CalcTextSize(buf);

        ImU32  text_color = IM_COL32(255, 255, 255, 255); /*black*/
        float x0 = first_tenured_dest_rect.x_lo();
        float x1 = last_tenured_dest_rect.x_hi();
        ImVec2 text_pos   = ImVec2(0.5 * (x0 + x1 - textz.x),
                                   tenured_dest_rect.y_mid() - 0.5 * textz.y);

        if (text_pos.x < x0 + 2)
            text_pos.x = x0 + 2;
        else if (text_pos.x < x0 + 0.5 * textz.x)
            text_pos.x = x0 + 0.5 * textz.x;

        draw_list->AddText(text_pos, text_color, buf);
    }
}

void
AnimateGcCopyCb::notify_gc_copy(std::size_t z,
                                const void * src_addr,
                                const void * dest_addr,
                                generation src_gen,
                                generation dest_gen)
{
    using xo::scope;
    using xo::xtag;
    using xo::gc::generation_result;
    using xo::gc::generation;
    using xo::gc::role;

    scope log(XO_DEBUG(false),
              xtag("z", z),
              xtag("src", src_addr),
              xtag("dest", dest_addr),
              xtag("src_gen", src_gen),
              xtag("dest_gen", dest_gen));

    auto [src_gen2, src_offset, src_alloc, src_size] = p_app_state_->gc_->fromspace_location_of(src_addr);

    if (src_gen2 == generation_result::not_found) {
        auto [lo, hi] = p_app_state_->gc_->nursery_span(role::from_space);

        log && log(xtag("N.from.lo", (void*)lo), xtag("N.from.hi", (void*)hi));

        assert(false);
    }

    generation src_valid_gen = xo::gc::valid_genresult2gen(src_gen2);

    auto [dest_gen2, dest_offset, _, dest_size] = p_app_state_->gc_->tospace_location_of(dest_addr);

    generation dest_valid_gen = xo::gc::valid_genresult2gen(dest_gen2);

    p_app_state_->copy_detail_v_.push_back(GcCopyDetail(z,
                                                        src_valid_gen, src_offset, src_alloc,
                                                        dest_valid_gen, dest_offset, dest_size));

    if (dest_valid_gen == generation::nursery) {
        p_app_state_->copy_detail_max_nursery_dest_offset_
            = std::max(p_app_state_->copy_detail_max_nursery_dest_offset_, dest_offset);
        p_app_state_->copy_detail_nursery_dest_size_
            = std::max(p_app_state_->copy_detail_nursery_dest_size_, dest_size);
    } else if (dest_valid_gen == generation::tenured) {
        p_app_state_->copy_detail_max_tenured_dest_offset_
            = std::max(p_app_state_->copy_detail_max_tenured_dest_offset_, dest_offset);
        p_app_state_->copy_detail_tenured_dest_size_
            = std::max(p_app_state_->copy_detail_tenured_dest_size_, dest_size);
    }

    /* will be animated across frames, see animate_gc_copy() */
}

int main(int, char **)
{
    using namespace std;

    scope log(XO_DEBUG(true));

    std::cout << "Hello, world!" << std::endl;

    SDL_SetHint(SDL_HINT_VIDEO_X11_FORCE_EGL, "0");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_version compiled;
    SDL_VERSION(&compiled);
    std::cerr << "SDL version: "
              << (int)compiled.major
              << "." << (int)compiled.minor
              << "." << (int)compiled.patch
              << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if 0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    std::cerr << "SDL video driver: " << SDL_GetCurrentVideoDriver() << std::endl;

    SDL_Window * window = SDL_CreateWindow("imgui + sdl2 + opengl",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           2000,
                                           1000,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window) {
        std::cerr << "SDL_CreateWindow done" << std::endl;
    } else {
        std::cerr << "SDL_CreateWindow failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    std::cerr << "Requested OpenGL vtersion: " << major << "." << minor << std::endl;

    if (gl_context) {
        std::cerr << "SDL_GL_CreateContext done" << std::endl;
    } else {
        std::cerr << "SDL_GL_CreateContext failed: [" << SDL_GetError() << "]" << std::endl;
        return -1;
    }

    if (SDL_GL_MakeCurrent(window, gl_context) != 0) {
        std::cerr << "SDL_GL_MakeCurrent failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(0); // disable vsync

    GLenum glew_status = glewInit();
    if (glew_status == GLEW_OK) {
        std::cerr << "glewInit done" << std::endl;
    } else {
        std::cerr << "glewInit failed: [" << glewGetErrorString(glew_status) << std::endl;
        return -1;
    }

    const GLubyte * version = glGetString(GL_VERSION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (version) {
        std::cerr << "OpenGL version: [" << version << "]" << std::endl;
    } else {
        std::cerr << "OpenGL version not available" << std::endl;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load noto sans font from unix environment NOTO_ONFTS_PATH
    // (see xo-umbrella2/default.nix shellHook)

    const char * fonts_path = std::getenv("DEJAVU_FONTS_PATH");

    if (fonts_path) {
        const float font_size = 14.0f;
        std::string font_path = xo::tostr(fonts_path, "/truetype/DejaVuSans.ttf");

        /* check file exists */
        std::ifstream font_in(font_path);
        if (font_in.good()) {
            std::cerr << "loading font [" << font_path << "]" << std::endl;
            ImFont * font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
            if (font) {
                std::cerr << "font loaded" << std::endl;

                ImFontConfig config;
                config.MergeMode = true;

                // latin extended chars
                static const ImWchar latin_ranges[] = {
                    0x0020, 0x00ff,  // basic latin + latin supplement
                    0x0100, 0x017f,  // latin extended-A
                    0x0180, 0x024f,  // latin extended-B
                    0x2080, 0x2099,  // subscript numerals + letters through n
                    0x25b2, 0x25b4,  // arrows
                    0,
                };
                io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size, &config, latin_ranges);
            } else {
                std::cerr << "font file load failed" << std::endl;
                std::cerr << "Fallback to default ImGui font" << std::endl;
            }
        }


    } else {
        std::cerr << "Expected DEJAVU_FONTS_PATH environment var." << std::endl;
        std::cerr << "Fallback to default ImGui font" << std::endl;
    }
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float counter_value = 0.0f;

    using xo::obj::Integer;
    using xo::obj::List;
    using xo::rng::xoshiro256ss;
    using xo::rng::Seed;
    using xo::up;
    using xo::gp;
    using xo::gc::GC;
    using xo::Object;

    AppState app_state;
    DrawState draw_state;
    /* note: during gc copy animation, this records state _before_ gc was triggered */
    GcStateDescription gcstate = app_state.snapshot_gc_state();

    app_state.gc_->add_gc_copy_callback(draw_state.make_gc_copy_animation(&app_state));

    // Main Loop
    bool done = false;

    while (!done) {
        /** on each draw cycle, app state falls into categories:
         *  1. allocation
         *     multiple draw cycles because many allocations per gc.
         *  2. garbage collection
         *     multiple draw cycles to animate copying process
         *     Settle conflict between {GC, imgui} as to who drives event loop,
         *     in favor of imgui; achieve this by copying what GC did,
         *     so that we can animate it over multiple draw cycles
         **/

        switch (draw_state.state_type_) {
        case draw_state_type::alloc:
        {
            /** generate random alloc **/
            app_state.generate_random_mutations();

            gcstate = app_state.snapshot_gc_state();

            app_state.upto_ = (app_state.gc_->is_full_gc_pending()
                               ? generation::tenured
                               : generation::nursery);

            /* GC may run here, in which case control reenters via AnimateGcCopyCb;
             * that callback captures copy details (per object!) in AppState
             */
            if (app_state.gc_->enable_gc_once()) {
                log && log(xtag("gc-type", (app_state.upto_ == generation::tenured) ? "full" : "incremental"));

                draw_state.state_type_ = draw_state_type::animate_gc;
                draw_state.animate_copy_t0_ = std::chrono::steady_clock::now();
            }

            break;
        }
        case draw_state_type::animate_gc:
        {
            /* don't update gcstate while animating,
             * that would use post-GC space sizing
             */

            break;
        }
        }

        /** poll + handle events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    if (event.window.windowID == SDL_GetWindowID(window))
                    {
                        done = true;
                    }
                } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // defer handling resize until device available
                    this->framebuffer_resized_flag_ = true;
                }
            }
        }

        //int w, h;
        //SDL_GetWindowSize(window, &w, &h);
        //glViewport(0, 0, w, h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw dear imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Background", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                     | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDecoration);
        ImGui::End();

        // 1. big demo window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. show window that we create ourselves
        {
            static int counter = 0;

            ImGui::Begin("Hello, world!");
            //ImGui::Text("This is totes useful text...");
            ImGui::Checkbox("demo window", &show_demo_window);
            //ImGui::Checkbox("second window", &show_another_window);

            ImGui::SliderInt("alloc/cycle", &app_state.alloc_per_cycle_, 1, 100);
            ImGui::SliderInt("copy animation budget", &draw_state.animate_copy_budget_ms_, 10, 10000);
            //ImGui::SliderFloat("alloc/cycle", &alloc_per_cycle, 0.0f, 1.0f);
            //ImGui::ColorEdit3("clear color", (float*)&clear_color);

            //if (ImGui::Button("Button"))
            //  ++counter;
            ImGui::NewLine(); // ImGui::SameLine()
            /* N\u2080 = N0, N\u2081 = N1 */
            ImGui::Text("alloc [%lu] avail [%lu] ",
                        gcstate.gc_allocated_,
                        gcstate.gc_available_);
            //ImGui::NewLine();
            ImGui::Text("promoted [%lu] copy animation [%lu / %lu]",
                        gcstate.total_promoted_,
                        static_cast<std::size_t>(draw_state.animate_copy_hi_pct_ * app_state.copy_detail_v_.size() / 100),
                        app_state.copy_detail_v_.size());

            ImGui::Text("mutation [%lu] mlog [%lu]",
                        gcstate.total_n_mutation_,
                        gcstate.gc_mlog_size_);

            ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                        1000.0f / io.Framerate, io.Framerate);

            ImGui::Text("layout:"
                        " nursery-src alloc rect [%.1f %.1f %.1f %.1f]"
                        " nursery-dest alloc rect [%.1f %.1f %.1f %.1f]"
                        " history rect [%.1f %.1f %.1f %.1f]",
                        draw_state.gcw_nursery_layout_.to_alloc_rect().x_lo(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().y_lo(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().x_hi(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().y_hi(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().x_lo(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().y_lo(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().x_hi(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().y_hi(),
                        draw_state.gcw_history_rect_.x_lo(),
                        draw_state.gcw_history_rect_.y_lo(),
                        draw_state.gcw_history_rect_.x_hi(),
                        draw_state.gcw_history_rect_.y_hi());
            ImGui::Text("nursery-dest copy offset [%lu] / size [%lu]"
                        " tenured-dest copy offset [%lu] / size [%lu]",
                        app_state.copy_detail_max_nursery_dest_offset_,
                        app_state.copy_detail_nursery_dest_size_,
                        app_state.copy_detail_max_tenured_dest_offset_,
                        app_state.copy_detail_tenured_dest_size_
                );

            ImDrawList * draw_list = ImGui::GetWindowDrawList();

            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            /* stash so GC copy animation can find it */
            draw_state.gcw_draw_list_ = draw_list;
            draw_state.gcw_canvas_p0_ = canvas_p0;
            draw_state.gcw_canvas_p1_ = canvas_p1;

            draw_gc_state(app_state,
                          gcstate,
                          ImRect(canvas_p0, canvas_p1),
                          draw_list,
                          &draw_state.gcw_nursery_layout_,
                          &draw_state.gcw_tenured_layout_,
                          //nullptr,
                          //&draw_state.gcw_tenured_alloc_rect_,
                          &draw_state.gcw_history_rect_);

            if (draw_state.state_type_ == draw_state_type::animate_gc) {
                auto animate_copy_t1 = std::chrono::steady_clock::now();
                auto animate_dt = animate_copy_t1 - draw_state.animate_copy_t0_;
                float animate_fraction_spent
                    = (std::chrono::duration_cast<std::chrono::milliseconds>(animate_dt).count()
                       / static_cast<float>(draw_state.animate_copy_budget_ms_));

                draw_state.animate_copy_hi_pct_ = 100.0 * animate_fraction_spent;
                animate_gc_copy(app_state,
                                draw_state,
                                draw_list);

                /* see 25.0 constant in animate_gc_copy() */
                if (draw_state.animate_copy_hi_pct_ >= 114) {
                    draw_state.state_type_ = draw_state_type::alloc;
                    draw_state.animate_copy_hi_pct_ = 0;
                    app_state.copy_detail_v_.clear();
                    app_state.copy_detail_max_nursery_dest_offset_ = 0;
                    app_state.copy_detail_nursery_dest_size_ = 0;
                    app_state.copy_detail_max_tenured_dest_offset_ = 0;
                    app_state.copy_detail_tenured_dest_size_ = 0;
                }
            }

            ImGui::End();
        }

        // 3. another window
        if (show_another_window) {
            ImGui::Begin("another window", &show_another_window);

            ImGui::Text("hello from second window");
            if (ImGui::Button("close me"))
                show_another_window = false;

            ImGui::End();
        }

        // rendering
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    std::cerr << "cleanup.." << std::endl;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cerr << "All done, goodbye..." << std::endl;

    return 0;
}

/* xo-imgui/example/ex1/imgui_ex2.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/object/Integer.hpp"
#include "xo/object/List.hpp"
#include "xo/alloc/GC.hpp"
#include "xo/alloc/Object.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/object/Integer.hpp"
#include "xo/flatstring/flatstring.hpp"
#include "xo/indentlog/scope.hpp"

#include <SDL.h>
#include <GL/glew.h>
#include <cstddef>

#include "SDL_events.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

//#include <GLFW/glfw3.h>

#ifdef NOPE
#include <SDL_opengl.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>

using xo::gc::generation;

ImVec2 operator+(const ImVec2 & p1, const ImVec2 & p2) {
    return ImVec2(p1.x + p2.x, p1.y + p2.y);
}

struct ImRect {
    ImRect() = default;
    ImRect(const ImVec2 & tl, const ImVec2 & br) : top_left_{tl}, bottom_right_{br} {}
    ImRect(float x_lo, float y_lo, float x_hi, float y_hi) : top_left_{x_lo, y_lo}, bottom_right_{x_hi, y_hi} {}

    static ImRect from_xy_span(const ImVec2 & x_span, const ImVec2 & y_span) {
        return ImRect(ImVec2{x_span.x, y_span.x}, ImVec2{x_span.y, y_span.y});
    }

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

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcGenerationDescription {
    GcGenerationDescription() = default;
    GcGenerationDescription(generation gen,
                            const char * name, const char * mnemonic, const char * gc_type,
                            std::uint8_t polarity,
                            std::size_t tospace_scale,
                            std::size_t before_ckp, std::size_t after_ckp,
                            std::size_t reserved, std::size_t committed, std::size_t gc_threshold)
        : name_{name}, mnemonic_{mnemonic}, gc_type_{gc_type}, polarity_{polarity},
          tospace_scale_{tospace_scale},
          before_checkpoint_{before_ckp}, after_checkpoint_{after_ckp},
          reserved_{reserved}, committed_{committed},
          gc_threshold_{gc_threshold} {}

    /** scale (in bytes) for drawing space **/
    std::size_t scale() const { return std::max(committed_, gc_threshold_); }

    /** nursery or tenured **/
    generation generation_;

    /** "nursery" or "tenured" **/
    const char * name_ = nullptr;

    /** "N" or "T" **/
    const char * mnemonic_ = nullptr;

    /** "incremental" or "full" **/
    const char * gc_type_ = nullptr;

    /** alternates between {0, 1} on each GC **/
    std::uint8_t polarity_ = 0;

    /** size of to-space in bytes represented on screen.
     *  (note however when we animate GC, space roles have already reversed,
     *   so then this will refer to old to-space = new from-space)
     **/
    std::size_t tospace_scale_ = 0;

    std::size_t before_checkpoint_ = 0;
    std::size_t after_checkpoint_ = 0;
    std::size_t reserved_ = 0;
    std::size_t committed_ = 0;
    // G_to_gc_threshold = G1_to_size + gc->config().incr_gc_threshold_;
    std::size_t gc_threshold_ = 0;
};

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcStateDescription {
    using generation = xo::gc::generation;

    GcStateDescription(const GcGenerationDescription & nursery,
                       const GcGenerationDescription & tenured,
                       std::size_t gc_size,
                       std::size_t gc_committed,
                       std::size_t gc_allocated,
                       std::size_t gc_available,
                       std::size_t gc_mlog_size,
                       std::size_t total_promoted,
                       std::size_t total_n_mutation
        );

    const GcGenerationDescription & get_gendescr(generation g) const { return gen_state_v_[gen2int(g)]; }

    std::array<GcGenerationDescription, static_cast<std::size_t>(generation::N)> gen_state_v_;

    /** see @ref GC::size **/
    std::size_t gc_size_ = 0;
    /** see @ref GC::committed **/
    std::size_t gc_committed_ = 0;
    /** see @ref GC::allocated **/
    std::size_t gc_allocated_ = 0;
    /** see @ref GC::available **/
    std::size_t gc_available_ = 0;
    /** see @ref GC::mlog_size **/
    std::size_t gc_mlog_size_ = 0;

    /** see @ref GcStatistics::total_promoted_ **/
    std::size_t total_promoted_ = 0;
    /** see @ref GcStatistics::n_mutation_ **/
    std::size_t total_n_mutation_ = 0;
};

GcStateDescription::GcStateDescription(const GcGenerationDescription & nursery,
                                       const GcGenerationDescription & tenured,
                                       std::size_t gc_size,
                                       std::size_t gc_committed,
                                       std::size_t gc_allocated,
                                       std::size_t gc_available,
                                       std::size_t gc_mlog_size,
                                       std::size_t total_promoted,
                                       std::size_t total_n_mutation)
    : gc_size_{gc_size},
      gc_committed_{gc_committed},
      gc_allocated_{gc_allocated},
      gc_available_{gc_available},
      gc_mlog_size_{gc_mlog_size},
      total_promoted_{total_promoted},
      total_n_mutation_{total_n_mutation}
{
    gen_state_v_[gen2int(generation::nursery)] = nursery;
    gen_state_v_[gen2int(generation::tenured)] = tenured;
}

using xo::gp;
using xo::up;
using xo::Object;
using xo::obj::List;
using xo::obj::Integer;
using xo::rng::xoshiro256ss;
using xo::rng::Seed;

/** details of a single copy event performed by GC **/
struct GcCopyDetail {
    GcCopyDetail(std::size_t z,
                 generation src, std::size_t src_offset, std::size_t src_space_z,
                 generation dest, std::size_t dest_offset, std::size_t dest_z)
        : z_{z},
          src_gen_{src}, src_offset_{src_offset}, src_space_z_{src_space_z},
          dest_gen_{dest}, dest_offset_{dest_offset}, dest_z_{dest_z}
        {}

    /** object size in bytes **/
    std::size_t z_ = 0;
    /** source location **/
    generation src_gen_;
    /** offset from start of allocator **/
    std::size_t src_offset_ = 0;
    /** size of source space.  could store this separately **/
    std::size_t src_space_z_ = 0;

    /** destination location **/
    generation dest_gen_;
    /** offset from start of allocator **/
    std::size_t dest_offset_ = 0;
    /** size of destination space. (could store this separately). **/
    std::size_t dest_z_ = 0;
};

struct AppState {
    using GC = xo::gc::GC;

public:
    AppState();

    std::size_t nursery_tospace_scale() const;
    std::size_t tenured_tospace_scale() const;
    GcStateDescription snapshot_gc_state() const;

    void generate_random_mutation();
    void generate_random_mutations();

public:
    int alloc_per_cycle_ = 1;
    /** if gc triggered, remembers which whether incremental or full **/
    generation upto_ = generation::nursery;
    up<GC> gc_;
    std::size_t next_int_ = 0;
    std::size_t next_root_ = 0;
    std::vector<gp<Object>> gc_root_v_{100};
    Seed<xoshiro256ss> seed_;
    xoshiro256ss rng_{seed_};
    /** remember details for each object copied by GC, so we can animate **/
    std::vector<GcCopyDetail> copy_detail_v_;
    /** max offset for destination, given copied to nursery **/
    std::size_t copy_detail_max_nursery_dest_offset_ = 0;
    std::size_t copy_detail_nursery_dest_size_ = 0;
    std::size_t copy_detail_max_tenured_dest_offset_ = 0;
    std::size_t copy_detail_tenured_dest_size_ = 0;
};

AppState::AppState()
{
    this->gc_ = (GC::make
           (
               {.initial_nursery_z_ = 1024*1024,
                .initial_tenured_z_ = 1024*1024*1024,
                .incr_gc_threshold_ = 16*1024,
                .full_gc_threshold_ = 128*1024,
                .stats_flag_ = true,
                .debug_flag_ = false}));

    Object::mm = gc_.get();

    for (auto & x: gc_root_v_)
        gc_->add_gc_root(x.ptr_address());

    gc_->disable_gc();
}

std::size_t
AppState::nursery_tospace_scale() const {
    std::size_t N1_to_size = gc_->nursery_before_checkpoint();
    std::size_t N_to_committed = gc_->nursery_to_committed();
    std::size_t N_to_incr_gc_threshold = N1_to_size + gc_->config().incr_gc_threshold_;
    std::size_t N_to_scale = std::max(N_to_committed, N_to_incr_gc_threshold);

    return N_to_scale;
}

std::size_t
AppState::tenured_tospace_scale() const {
    std::size_t T1_to_size = gc_->tenured_before_checkpoint();
    std::size_t T_to_committed = gc_->tenured_to_committed();
    std::size_t T_to_full_gc_threshold = T1_to_size + gc_->config().full_gc_threshold_;
    std::size_t T_to_scale = std::max(T_to_committed, T_to_full_gc_threshold);

    return T_to_scale;
}

GcStateDescription
AppState::snapshot_gc_state() const {
    /** NOTE: this gets invoked before GC gets opportunity to run.
     *        in the event that GC does run, from- and to- spaces will
     *        have been reversed (near beginning of GC phase)
     *
     *        This means that nursery_to_reserved() etc. actually refer to from-space
     *        *during gc*
     **/

    // TOOD: may want to use GC::get_gc_statistics() to replace multiple round trips

    return GcStateDescription(GcGenerationDescription
                              (generation::nursery,
                               "nursery",
                               "N",
                               "incremental",
                               gc_->nursery_polarity(),
                               this->nursery_tospace_scale(),
                               gc_->nursery_before_checkpoint(),
                               gc_->nursery_after_checkpoint(),
                               gc_->nursery_to_reserved(),
                               gc_->nursery_to_committed(),
                               gc_->nursery_before_checkpoint() + gc_->config().incr_gc_threshold_),
                              GcGenerationDescription
                              (generation::tenured,
                               "tenured",
                               "T",
                               "full",
                               gc_->tenured_polarity(),
                               this->tenured_tospace_scale(),
                               gc_->tenured_before_checkpoint(),
                               gc_->tenured_after_checkpoint(),
                               gc_->tenured_to_reserved(),
                               gc_->tenured_to_committed(),
                               gc_->tenured_before_checkpoint() + gc_->config().full_gc_threshold_),

                              gc_->size(),
                              gc_->committed(),
                              gc_->allocated(),
                              gc_->available(),
                              gc_->mlog_size(),
                              gc_->native_gc_statistics().total_promoted_,
                              gc_->native_gc_statistics().n_mutation_
        );
}

void
AppState::generate_random_mutation() {
    if (rng_() % 1000 > (5 * 1000) / 7) {
        /* p=16% integer */
        gc_root_v_[next_root_++] = Integer::make(next_int_);
    } else if (rng_() % 1000 > (3 * 1000) / 7) {
        /* p=16% cons */
        gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
        if (random_car.is_null())
            random_car = List::nil;

        /* this will always incorporate existing list as tail of new list */
        gp<List> random_cdr = List::from(gc_root_v_[next_root_]);
        if (random_cdr.is_null())
            random_cdr = List::nil;

        gp<List> random_cons = List::cons(random_car, random_cdr);

        gc_root_v_[next_root_++] = random_cons;
    } else if (rng_() % 1000 > (0 * 1000) / 7) {
        /* p=24% mutation */
        gp<List> random_list = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
        if (!random_list.is_null()) {
            if (rng_() % 2 == 0) {
                /* pick up some random object, assign as head */
                gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
                random_list->assign_head(random_car);
            } else {
                /* pick up some random object; if List, assign tail as tail */
                gp<List> random_cdr = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
                if (!random_cdr.is_null() && !random_cdr->is_nil())
                    random_list->assign_rest(random_cdr->rest());
            }
        }
    }
    if (next_root_ >= gc_root_v_.size())
        this->next_root_ = 0;
}

void
AppState::generate_random_mutations() {
    for (int i = 0; i < this->alloc_per_cycle_; ++i) {
        this->generate_random_mutation();
    }
}

void
draw_filled_rect_with_label(const char * text,
                            const char * tooltip,
                            const ImRect & rect,
                            ImU32 fillcolor,
                            ImU32 textcolor,
                            ImDrawList * draw_list)
{
    draw_list->AddRectFilled(rect.top_left(),
                             rect.bottom_right(),
                             fillcolor);

    if ((rect.width() > 0.0) && (rect.height() > 0.0)) {
        ImGui::SetCursorScreenPos(rect.top_left());
        ImGui::InvisibleButton("ttbutton", ImVec2(rect.width(), rect.height()));
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }
    }

    if (text) {
        auto textz = ImGui::CalcTextSize(text);

        /* N1 can be empty: but in that case don't bother to label it */
        if (textz.x < rect.width()) {
            draw_list->AddText(ImVec2(rect.x_mid() - 0.5 * textz.x,
                                      rect.y_mid() - 0.5 * textz.y),
                               textcolor,
                               text);
        }
    }
}

void
draw_filled_rect(const char * tooltip,
                 const ImRect & rect,
                 ImU32 fillcolor,
                 ImDrawList * draw_list)
{
    draw_filled_rect_with_label(nullptr,
                                tooltip,
                                rect,
                                fillcolor,
                                IM_COL32(255, 255, 255, 255),
                                draw_list);
}


using xo::gc::GC;
using xo::gc::GcStatisticsExt;
using xo::gc::GcStatisticsHistory;
using xo::gc::GcStatisticsHistoryItem;
using xo::xtag;
using xo::scope;
using xo::flatstring;
using std::size_t;

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
    flatstring<80> rh_text_;

    /** width of .rh_text in screen units **/
    float rh_text_dx_ = 0.0;

    /** rectangle representing from-space memory range **/
    ImRect mem_rect_from_;
    /** rectangle representing to-space memory range **/
    ImRect mem_rect_to_;
};

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

/**
 *  @p polarity  0 -> draw from-space above to-space; 1 -> draw from-space below to-space
 *  @p p_x1    On exit *p_x1 contains x-coord of right-hand edge of rectangle
 *             depicting potential memory range
 **/
void
draw_generation(const GenerationLayout & layout,
                ImDrawList * draw_list)
{
    //scope log(XO_DEBUG(with_labels));

    using xo::gc::generation;

    /* next GC trigges when G0_to_size reaches this threshold */
    std::size_t G_to_gc_threshold = 0;

    G_to_gc_threshold = layout.gendescr_.gc_threshold_;

    std::size_t G_to_scale = layout.gendescr_.scale();

    /*
     * committed: G_to_committed
     * G1:  G1_to_size
     * G0:  G0_to_size
     * ckp: G1_to_size
     * ngc: G_to_gc_threshold
     *
     *   <----------------------------- committed --------------------------->
     *   <------------------ used ------------------> <-------- free -------->
     *   <------- G1 --------> <-------- G0 -------->
     *  |NNNNNNNNNNNNNNNNNNNNN|nnnnnnnnnnnnnnnnnnnnnn|________________________|
     *                        ^                                      ^
     *                        ckp                                    ngc
     *
     *  in screen coords:
     *
     *  horizontally:
     *
     *         rect.x_lo                                                                   rect.x_hi
     *         v                                                                           v
     *
     *          <--------------------------- display_w -----------------------------> <-+->
     *          <------------------------ ngc_w ---------------------->                 \- rh_text_dx
     *          <------- G1_w ------> <-------- G0_w ------>
     *         ^                     ^                      ^          ^             ^
     *        x0                    G0_x0                  G0_x1     ngc_w          x1
     *     *p_x0                                        *p_g0_x1                 *p_x1
     *
     *  vertically:
     *
     *      <- rect.y_lo
     *     ^
     *
     *     v
     *      <- rect.y_hi
     */

    /* e.g. N1: 34511 bytes */
    char g1_buf[255];

    ImU32 label_color = IM_COL32(255, 255, 192, 255); /*super pale yellow*/

    if (layout.with_labels_) {
        snprintf(g1_buf, sizeof(g1_buf),
                 "reserved: %lu bytes; committed: %lu bytes; %s\u2081: %lu bytes; %s\u2080: %lu bytes",
                 layout.gendescr_.reserved_,
                 layout.gendescr_.committed_,
                 layout.mnemonic(),
                 layout.gendescr_.before_checkpoint_,
                 layout.mnemonic(),
                 layout.gendescr_.after_checkpoint_);

        draw_list->AddText(layout.bounding_rect_.top_left(),
                           label_color,
                           g1_buf);
    }

    if (layout.with_labels_) {
        auto textz = ImGui::CalcTextSize(layout.rh_text_.c_str());

        draw_list->AddText(ImVec2(layout.chart_withlabel_rect_.x_hi() - textz.x,
                                  layout.chart_withlabel_rect_.y_mid() - 0.5 * textz.y),
                           label_color,
                           layout.rh_text_.c_str());
    }

    ImU32 outline_color = IM_COL32(255, 255, 255, 255); /*white*/

    /* chart rectangle */
    draw_list->AddRect(layout.mem_rect_from_.top_left(),
                       layout.mem_rect_from_.bottom_right(),
                       outline_color);

    draw_list->AddRect(layout.mem_rect_to_.top_left(),
                       layout.mem_rect_to_.bottom_right(),
                       outline_color);

    float display_w = layout.mem_rect_from_.width();
    float G1_w = display_w * layout.to_G1_size() / layout.to_scale();
    float G1_x1 = layout.mem_rect_from_.x_lo() + G1_w;
    ImRect G1_rect = layout.to_g1_rect();
    //ImRect G1_rect = layout.mem_rect_to_.left_fraction(layout.to_G1_size() / layout.to_scale());

    /* G1 (i.e. N1 or T1) */
    {
        ImU32   G1_color = IM_COL32(  0, 128,   0, 255);
        ImU32 text_color = IM_COL32(255, 255, 255, 255);

        char buf[255];

        if (layout.with_labels_)
            snprintf(buf, sizeof(buf), "%s\u2081: %luk",
                     layout.mnemonic(), layout.to_G1_size() / 1024); /* N1 / T1 */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2081: %lu - %s survivor size in bytes",
                 layout.mnemonic(),
                 layout.to_G1_size(),
                 layout.name());

        draw_filled_rect_with_label(layout.with_labels_ ? buf : nullptr,
                                    tooltip,
                                    G1_rect,
                                    G1_color,
                                    text_color,
                                    draw_list);
    }

    ImRect G0_rect = layout.to_g0_rect();

    /* G0 (i.e. N0 or T0) */
    {
        ImU32   G0_color = IM_COL32( 32, 192,  32, 255);
        ImU32 text_color = IM_COL32(  0,   0,   0, 255);

        char buf[255];

        if (layout.with_labels_)
            snprintf(buf, sizeof(buf), "%s\u2080: %luk",
                     layout.mnemonic(),
                     layout.to_G0_size() / 1024); /* N(0) */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2080: %lu - %s new alloc size in bytes",
                 layout.mnemonic(), layout.to_G0_size(),
                 layout.name());

        draw_filled_rect_with_label(layout.with_labels_ ? buf : nullptr,
                                    tooltip,
                                    G0_rect,
                                    G0_color,
                                    text_color,
                                    draw_list);
    }

    /* mark where next gc will trigger */
    if (layout.with_labels_) {
        const char * uparrow = reinterpret_cast<const char *>(u8"\u25b3");

        float ngc_w = (display_w * layout.gendescr_.gc_threshold_) / G_to_scale;

        auto tmp = ImGui::CalcTextSize(uparrow);
        std::size_t uparrow_w = tmp.x;
        double ngc_x = layout.chart_withlabel_rect_.x_lo() + ngc_w - uparrow_w/2.0;

        ImVec2 marker_pos(ngc_x, layout.chart_withlabel_rect_.y_hi());

        draw_list->AddText(marker_pos,
                           IM_COL32(255, 128, 128, 255) /*red*/,
                           uparrow);

        ImGui::SetCursorScreenPos(marker_pos);
        ImGui::InvisibleButton("mkbutton", tmp);
        if (ImGui::IsItemHovered()) {
            char marker_tt_buf[255];
            snprintf(marker_tt_buf, sizeof(marker_tt_buf),
                     "Next %s GC when size(%s) >= %lu bytes",
                     layout.gc_type(),
                     layout.name(),
                     layout.gendescr_.gc_threshold_);

            ImGui::SetTooltip("%s", marker_tt_buf);
        }
    }
} /*draw_generation*/

void
draw_nursery(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             GenerationLayout * p_layout)
{
    using xo::gc::generation;

    const GcGenerationDescription & gendescr = gcstate.get_gendescr(generation::nursery);

    GenerationLayout layout(gendescr, rect, with_labels);

    draw_generation(layout, draw_list);

    if (p_layout)
        *p_layout = layout;
}

void
draw_tenured(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             GenerationLayout * p_layout)
{
    using xo::gc::generation;

    const GcGenerationDescription & gendescr = gcstate.get_gendescr(generation::tenured);

    GenerationLayout layout(gendescr, rect, with_labels);

    draw_generation(layout, draw_list);

    if (p_layout)
        *p_layout = layout;
}

/** for history tooltip, choose which statistic to headline **/
enum class gc_history_headline {
    survive,
    promote,
    persist,
    garbage0,
    garbage1,
    garbageN,
    N
};

xo::flatstring<512>
write_gc_history_tooltip(gc_history_headline headline,
                         const GcStatisticsHistoryItem & stats)
{
    xo::flatstring<512> retval;

    xo::flatstring<512> headline_str;
    switch (headline) {
    case gc_history_headline::survive:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "survive: %lu: bytes surviving 1st GC after allocation",
                 stats.survive_z_);
        break;
    case gc_history_headline::promote:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "promote: %lu: bytes surviving 2nd GC; if nursery promote to tenured",
                 stats.promote_z_);
        break;
    case gc_history_headline::persist:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "persist: %lu: bytes surviving 3+ GCs. Only non-zero for full collections",
                 stats.persist_z_);
        break;
    case gc_history_headline::garbage0:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2080: %lu: bytes collected on 1st GC after allocation",
                 stats.garbage0_z_);
        break;
    case gc_history_headline::garbage1:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2081: %lu: bytes collected on 2nd GC after allocation",
                 stats.garbage1_z_);
        break;
    case gc_history_headline::garbageN:
        snprintf(headline_str.data(), headline_str.capacity(),
                 "garbage\u2099: %lu: bytes collected on 3rd or later GC after allocation",
                 stats.garbageN_z_);
        break;
    case gc_history_headline::N:
        assert(false);
        break;
    }



    snprintf(retval.data(), retval.capacity(),
             "%s\n"
             "\n"
             " gcseq: %lu\n"
             " type: %s\n"
             " alloc: %lu\n"
             " survive: %lu\n"
             " promote: %lu\n"
             " persist: %lu\n"
             " garbage\u2080: %lu\n" /*garbage0*/
             " garbage\u2081: %lu\n" /*garbage1*/
             " garbage\u2099: %lu\n" /*garbageN*/
             " effort: %lu dt: %.1lfus\n"
             " copy efficiency: %.1lf%% collection rate: %.0lf bytes/sec",
             headline_str.c_str(),
             stats.gc_seq_,
             (stats.upto_ == generation::nursery) ? "incremental" : "FULL",
             stats.new_alloc_z_,
             stats.survive_z_,
             stats.promote_z_,
             stats.persist_z_,
             stats.garbage0_z_,
             stats.garbage1_z_,
             stats.garbageN_z_,
             stats.effort_z_,
             1e-3 * stats.dt_.scale(),
             100.0 * stats.efficiency(),
             stats.collection_rate()
        );

    return retval.ensure_final_null();
}

/** stacked bar chart
 *
 *  @param gen  if @ref generation::nursery, only display nursery collections.
 *              otherwise display both
 **/
void
draw_gc_history(const GcStateDescription & gcstate,
                generation gen,
                const GcStatisticsHistory & gc_history,
                const ImRect & bounding_rect,
                bool debug_flag,
                ImDrawList * draw_list)
{
    scope log(XO_DEBUG(debug_flag));

    float lm = 10;
    float tm = 25;

    /* we're going to make a bar chart */

    /* x_scale,y_scale in GC units (i.e. bytes) */
    size_t x_scale = gc_history.capacity();
    size_t yplus_scale = 0;
    size_t yminus_scale = 0;

    float display_w = bounding_rect.width()  - lm;
    float display_h = bounding_rect.height() - tm;

    /* 1st loop: figure out max y scale */
    for (const GcStatisticsHistoryItem & stats : gc_history) {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            //size_t  na = stats.new_alloc_z_ - stats.survive_z_; /*new allocs, but dont' double-count survive_z*/
            size_t  sz = stats.survive_z_; /*survive 1st gc */
            size_t  pz = stats.promote_z_; /*survive 2nd gc */
            size_t psz = stats.persist_z_; /*survive 3+ gc */
            size_t g0z = stats.garbage0_z_;
            size_t g1z = stats.garbage1_z_;
            size_t gNz = stats.garbageN_z_;

            if (yplus_scale < sz + pz + psz)
                yplus_scale = sz + pz + psz;

            if (yminus_scale < g0z + g1z + gNz)
                yminus_scale = g0z + g1z + gNz;
        } else {
            ;
        }
    }

    /* y-coord of x-axis */
    float y_zero = bounding_rect.y_lo() + tm + (display_h * yplus_scale) / (yplus_scale + yminus_scale);

    float y_scale = yplus_scale + yminus_scale;

    /* width of 1 bar in screen coords */
    constexpr float c_min_bar_w = 5.0;
    float bar_w = std::max(c_min_bar_w, display_w / gc_history.capacity());

    /* 2nd loop: draw bars */
    std::size_t i = 0;
    for (const GcStatisticsHistoryItem & stats : gc_history)
    {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            /*
             *     ys_lo   +--+
             *             |  |   survive_z  (survived 1st GC)
             *             |  |
             *     yp_lo   +--+
             *             |  |   promote_z  (sruvived 2nd GC)
             *             |  |
             *   ypsz_lo   +--+
             *             |  |   persist_z  (survived 3+ GCs)
             *             |  |
             *    y_zero   +--+
             *             |  |   gN  (killed on 3+ GC)
             *             |  |
             *    ygN_hi   +--+
             *             |  |   g1  (killed on 2nd GC)
             *             |  |
             *    yg1_hi   +--+
             *             |  |   g0  (killed on 1st GC)
             *             |  |
             *    yg0_hi   +--+
             */

            ImU32  persist_color = IM_COL32(  0,  64, 192, 255); /*darker blue*/
            ImU32  promote_color = IM_COL32(  0, 128,   0, 255); /*darker green*/
            ImU32  survive_color = IM_COL32( 32, 192,  32, 255); /*lighter green*/
            ImU32 garbageN_color = IM_COL32(255, 128,  64, 255); /*darker orange*/
            ImU32 garbage1_color = IM_COL32(255, 192, 128, 255); /*medium orange*/
            ImU32 garbage0_color = IM_COL32(255, 255, 192, 255); /*pale yellow*/

            /* x-coordinates of bar */
            float x_lo = bounding_rect.x_lo() + lm + i * bar_w;
            float x_hi = x_lo + bar_w - 1;
            ImVec2 x_span{x_lo, x_hi};

            /* y-coordinates of persist bar (survived 3+ GCs) */
            float ypsz_lo = (y_zero
                             - (display_h * stats.persist_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::persist, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ypsz_lo, y_zero)),
                                 persist_color,
                                 draw_list);
            }
            /* y-coordinates of promote bar (survived 2nd GC) */
            float yp_hi = ypsz_lo;
            float yp_lo = (yp_hi
                           - (display_h * stats.promote_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::promote, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(yp_lo, yp_hi)),
                                 promote_color,
                                 draw_list);
            }

            /* y-coordinates of survivor bar (survived 1st GC) */
            float ys_hi = yp_lo;
            float ys_lo = (ys_hi - (display_h * stats.survive_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::survive, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ys_lo, ys_hi)),
                                 survive_color,
                                 draw_list);
            }

            // -----------------------------------------------------------

            /* y-coordinates of garbageN bar (killed on 3+ GC) */
            float ygN_lo = y_zero;
            float ygN_hi = (y_zero
                            + (display_h * stats.garbageN_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbageN, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect::from_xy_span(x_span, ImVec2(ygN_lo, ygN_hi)),
                                 garbageN_color,
                                 draw_list);
            }

            /* y-coordinates of garbage1 bar (killed on 2nd GC) */
            float yg1_lo = ygN_hi;
            float yg1_hi = (yg1_lo
                            + (display_h * stats.garbage1_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbage1, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect(ImVec2(x_lo, yg1_lo), ImVec2(x_hi, yg1_hi)),
                                 garbage1_color,
                                 draw_list);
            }

            /* y-coordinates of garbage0 bar (killed on 1st GC) */
            float yg0_lo = yg1_hi;
            float yg0_hi = (yg0_lo
                            + (display_h * stats.garbage0_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::garbage0, stats);

                draw_filled_rect(tt.c_str(),
                                 ImRect(ImVec2(x_lo, yg0_lo), ImVec2(x_hi, yg0_hi)),
                                 garbage0_color,
                                 draw_list);
            }
        } else {
            /* draw nothing */
            ;
        }

        ++i;
    }

    log && log(xtag("i", i));
}

void
draw_gc_efficiency(const GcStateDescription & gcstate,
                   //generation gen,
                   const GcStatisticsHistory & gc_history,
                   const ImRect & bounding_rect,
                   bool debug_flag,
                   ImDrawList * draw_list)
{
    scope log(XO_DEBUG(debug_flag));

    float lm = 10;
    float tm = 25;

    /* we're going to make a level chart */

    /* x_scale,y_scale in GC units (i.e. bytes) */
    size_t x_scale = gc_history.capacity();
    size_t yplus_scale = 1;
    size_t yminus_scale = 0;

    float display_w = bounding_rect.width()  - lm;
    float display_h = bounding_rect.height() - tm;

#ifdef NOPE // don't need this.  y-scale is [0.0, 1.0]
    /* 1st loop: figure out max y scale */
    for (const GcStatisticsHistoryItem & stats : gc_history) {
        if ((gen == stats.upto_) || (gen == generation::tenured))
        {
            //size_t  na = stats.new_alloc_z_ - stats.survive_z_; /*new allocs, but dont' double-count survive_z*/
            size_t  sz = stats.survive_z_; /*survive 1st gc */
            size_t  pz = stats.promote_z_; /*survive 2nd gc */
            size_t psz = stats.persist_z_; /*survive 3+ gc */
            size_t g0z = stats.garbage0_z_;
            size_t g1z = stats.garbage1_z_;
            size_t gNz = stats.garbageN_z_;

            if (yplus_scale < sz + pz + psz)
                yplus_scale = sz + pz + psz;

            if (yminus_scale < g0z + g1z + gNz)
                yminus_scale = g0z + g1z + gNz;
        } else {
            ;
        }
    }
#endif

    /* y-coord of x-axis */
    float y_zero = bounding_rect.y_lo() + tm + display_h;
    //float y_zero = bounding_rect.y_lo() + tm + (display_h * yplus_scale) / (yplus_scale + yminus_scale);

    float y_scale = 1.0;

    /* width of 1 bar in screen coords */
    constexpr float c_min_bar_w = 5.0;
    float bar_w = std::max(c_min_bar_w, display_w / gc_history.capacity());

    /* TODO: use temporary arena */
    std::vector<ImVec2> line_points;
    line_points.reserve(gc_history.size());

    ImU32 average_color = IM_COL32(255, 255,  64, 255); /*solid yellow*/
    ImU32  sample_color = IM_COL32(255, 255, 255, 255); /*white*/

    /* 2nd loop: draw levels */
    std::size_t i = 0;
    for (const GcStatisticsHistoryItem & stats : gc_history)
    {
        //std::vector<ImVec2> line_points = { /* your points */ };
        //draw_list->AddPolyline(line_points.data(), line_points.size(),
        //                       IM_COL32(255, 255, 0, 255), false, 2.0f);

        float y = y_zero - display_h * stats.efficiency();
        float y_mean = y_zero - display_h * stats.average_efficiency();

        /* x-coordinates of point */
        float x = bounding_rect.x_lo() + lm + i * bar_w + 0.5 * bar_w;

        line_points.push_back(ImVec2(x, y_mean));

        draw_list->AddCircleFilled(ImVec2(x, y), 2.0f, sample_color);

        ++i;
    }

    draw_list->AddPolyline(line_points.data(),
                           line_points.size(),
                           average_color,
                           false,
                           1.0f /*line width?*/);

    log && log(xtag("i", i));
} /*draw_gc_efficiency*/


void
draw_gc_alloc_state(const GcStateDescription & gcstate,
                    const ImRect & canvas_rect,
                    ImDrawList * draw_list,
                    GenerationLayout * p_nursery_layout,
                    GenerationLayout * p_tenured_layout
                    //ImRect * p_nursery_alloc_rect,
                    //ImRect * p_tenured_alloc_rect
    )
{
    constexpr float c_est_chart_text_height = 14.0;
    constexpr float c_min_h =  7;  // chart bar height
    constexpr float c_max_h = 40;  // chart bar height

    /* bounding rectange for nursery display */
    ImRect N_space_rect = canvas_rect.top_fraction(0.5,
                                                   c_min_h + c_est_chart_text_height,
                                                   c_max_h + c_est_chart_text_height);

    //assert(N_space_rect.height() >= c_min_h + c_est_chart_text_height);
    //assert(N_space_rect.height() <= c_max_h + c_est_chart_text_height);

    /* rectangle representing allocated nursery range */
    //float N_x1 = 0.0;
    GenerationLayout N_layout;

    draw_nursery(gcstate,
                 true /*with_labels*/,
                 N_space_rect,
                 draw_list,
                 &N_layout);

    float N_x1 = N_layout.chart_nolabel_rect_.x_hi();

    if (p_nursery_layout)
        *p_nursery_layout = N_layout;

//    if (p_nursery_alloc_rect)
//        *p_nursery_alloc_rect = N_layout.to_alloc_rect();

    /* N0_to_size..N_to_scale: in bytes */
    std::size_t N_to_scale = gcstate.gen_state_v_[gen2int(generation::nursery)].tospace_scale_;

    /* display_w .. N0_h : viewportcoords */
    std::size_t display_w = canvas_rect.width();

    std::size_t x0 = canvas_rect.x_lo();
    std::size_t x1 = canvas_rect.x_hi();

    // now turn to Tenured space

    std::size_t T_to_scale = gcstate.gen_state_v_[gen2int(generation::tenured)].tospace_scale_;

    /* want to put to-scale image of nursery next to to-scale image of tenured;
     * but also want space between them.
     */
    float TplusN_to_scale = N_to_scale + T_to_scale;
    /* space between T, N images */
    float TplusN_spacer = 10;

    /* bounding rectange for tenured display */
    ImRect T_space_rect = (canvas_rect
                           .within_bottom_margin(c_est_chart_text_height)
                           .bottom_fraction(0.5,
                                            c_min_h + c_est_chart_text_height,
                                            c_max_h + c_est_chart_text_height));

    if (N_space_rect.y_hi() > T_space_rect.y_lo()) {
        T_space_rect = T_space_rect.translate(ImVec2(0, N_space_rect.y_hi() - T_space_rect.y_lo()));
    }

    assert(T_space_rect.y_lo() >= N_space_rect.y_hi());

    /* for smaller image of nursery */
    //std::size_t t_y0 = canvas_rect.y_lo() + 70 + alloc_height + 20;

    /* for side-by-side tenured + nursery, with both on same scale
     * 2nd term is horiz space used for N label like 'Mem: 28k'
     */
    std::size_t adj_display_w = display_w - (N_space_rect.x_hi() - N_x1);

    /* bounding rectangle for secondary nursery display */
    ImRect np_rect(ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)),
                          T_space_rect.y_lo() + c_est_chart_text_height),
                   ImVec2(x0 + adj_display_w,
                          T_space_rect.y_hi()));

    // redraw nursery to same scale as tenured
    {
        draw_list->AddLine(N_space_rect.bottom_left(), np_rect.top_left(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);
        draw_list->AddLine(ImVec2(N_x1, N_space_rect.y_hi()), np_rect.top_right(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);

        draw_nursery(gcstate,
                     false /*no labels*/,
                     np_rect,
                     draw_list,
                     nullptr);
    }

    /* rectangle representing allocated tenured range */
    GenerationLayout T_layout;

    draw_tenured(gcstate,
                 true /*with labels*/,
                 ImRect(ImVec2(x0, T_space_rect.y_lo()),
                        ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)) - TplusN_spacer,
                               T_space_rect.y_hi())),
                 draw_list,
                 &T_layout);

    if (p_tenured_layout)
        *p_tenured_layout = T_layout;

//    if (p_tenured_alloc_rect)
//        *p_tenured_alloc_rect = T_layout.to_alloc_rect();

} /*draw_gc_alloc_state*/

void
draw_gc_state(const AppState & app_state,
              const GcStateDescription & gcstate,
              const ImRect & canvas_rect,
              ImDrawList * draw_list,
              GenerationLayout * p_nursery_layout,
              GenerationLayout * p_tenured_layout,
              //ImRect * p_nursery_alloc_rect,
              //ImRect * p_tenured_alloc_rect,
              ImRect * p_history_rect)
{
    // draw stuff
    draw_list->AddRect(canvas_rect.top_left(),
                       canvas_rect.bottom_right(),
                       IM_COL32(255, 255, 255, 255));

    /* TODO: does this reset coord space? */
    ImRect alloc_rect;
    {
        ImGui::BeginChild("top pane", ImVec2(0, 105), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);

        alloc_rect = ImRect(canvas_rect.top_left() + ImGui::GetWindowContentRegionMin(),
                            canvas_rect.top_left() + ImGui::GetWindowContentRegionMax());
        ImRect draw_rect = alloc_rect.within_margin(ImRect(50, 10, 70, 10));

        draw_list->PushClipRect(draw_rect.top_left(), draw_rect.bottom_right());

        draw_gc_alloc_state(gcstate,
                            draw_rect,
                            draw_list,
                            p_nursery_layout,
                            p_tenured_layout
                            //p_nursery_alloc_rect,
                            //p_tenured_alloc_rect
            );

        draw_list->PopClipRect();

        ImGui::EndChild();
    }

    ImRect history_rect;
    {
        ImGui::BeginChild("left pane", ImVec2(800, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

        /* history below alloc area */
        history_rect = ImRect(alloc_rect.bottom_left() + ImGui::GetWindowContentRegionMin(),
                              alloc_rect.bottom_left() + ImGui::GetWindowContentRegionMax());

         if (p_history_rect)
             *p_history_rect = history_rect;

         draw_list->PushClipRect(history_rect.top_left(), history_rect.bottom_right());

         float lm = 50;
         float rm = 70;
         float tm = 10;
         std::size_t x0 = history_rect.x_lo() + lm;
         std::size_t x1 = history_rect.x_hi() - rm;
         std::size_t h_y0 = history_rect.y_lo() + tm;

         /* just incremental (nursery) collections */
         ImRect incremental_rect = history_rect.top_fraction(0.33);

         draw_gc_history(gcstate,
                         generation::nursery,
                         app_state.gc_->gc_history(),
                         incremental_rect,
                         false /*debug_flag*/,
                         draw_list);

         /* just full (nursery+tenured) collections */
         ImRect full_rect = history_rect.bottom_fraction(0.67).top_fraction(0.5);

         /* both nursery + full collections */
         draw_gc_history(gcstate,
                         generation::tenured,
                         app_state.gc_->gc_history(),
                         full_rect,
                         false /*debug_flag*/,
                         draw_list);

         ImRect efficiency_rect = history_rect.bottom_fraction(0.67).bottom_fraction(0.5);

         draw_gc_efficiency(gcstate,
                            app_state.gc_->gc_history(),
                            efficiency_rect,
                            false /*debug_flag*/,
                            draw_list);

         draw_list->PopClipRect();

         ImGui::EndChild();
    }

    ImGui::Text("placeholder text");

    /* BeginChild() again ? */

#ifdef NOPE
    draw_list->AddCircleFilled(ImVec2(canvas_p0.x + 50, canvas_p0.y + 50),
                               30.0f, IM_COL32(255, 0, 0, 255));

    draw_list->AddText(ImVec2(canvas_p0.x + 10, canvas_p0.y + 10),
                       IM_COL32(255, 255, 255, 255), "Hello 2D!");
#endif
}

struct DrawState;

struct AnimateGcCopyCb : public xo::gc::GcCopyCallback {
    using generation = xo::gc::generation;

    explicit AnimateGcCopyCb(AppState * appstate, DrawState * drawstate)
        : p_app_state_{appstate}, p_draw_state_{drawstate} {}

    virtual void notify_gc_copy(std::size_t z,
                                const void * src_addr, const void * dest_addr,
                                generation src_gen, generation dest_gen);

    AppState * p_app_state_ = nullptr;
    DrawState * p_draw_state_ = nullptr;
};

enum class draw_state_type {
    alloc,
    animate_gc
};

struct DrawState {
    up<xo::gc::GcCopyCallback> make_gc_copy_animation(AppState * app_state) {
        return std::make_unique<AnimateGcCopyCb>(app_state, this);
    }

    draw_state_type state_type_ = draw_state_type::alloc;

    /** budgeted time period over which to animate gc copy **/
    int animate_copy_budget_ms_ = 2000;
    /** start time of current copy animation **/
    std::chrono::steady_clock::time_point animate_copy_t0_;
    /** when animating copy step, display objects from AppState::copy_detail_v_[i]
     *  where i < .animate_copy_hi_ / 100 * AppState::copy_detail_v_.size()
     **/
    float animate_copy_hi_pct_ = 0;

    ImDrawList * gcw_draw_list_ = nullptr;

    /** draw area **/
    ImVec2 gcw_canvas_p0_;
    ImVec2 gcw_canvas_p1_;

    /** layout for nursery display **/
    GenerationLayout gcw_nursery_layout_;
    /** layout for tenured display **/
    GenerationLayout gcw_tenured_layout_;

    /** rect displaying gc history (strip charts) **/
    ImRect gcw_history_rect_;
};

ImRect map_src_alloc_to_screen(const GcCopyDetail & copy_detail,
                               const ImRect & space_rect)
{
    // TODO: methods on copy_detail / and/or ImPoint

    auto [x_coord_lo, x_coord_hi] = space_rect.x_span();

    double w0 = copy_detail.src_offset_ / static_cast<double>(copy_detail.src_space_z_);
    float src0_x = ((1.0 - w0) * x_coord_lo) + (w0 * x_coord_hi);

    double w1 = ((copy_detail.src_offset_ + copy_detail.z_)
                 / static_cast<double>(copy_detail.src_space_z_));
    float src1_x = ((1.0 - w1) * x_coord_lo) + (w1 * x_coord_hi);

    return space_rect.with_x_span(src0_x, src1_x);
}

ImRect map_dest_alloc_to_screen(const GcCopyDetail & copy_detail,
                                const ImRect & space_rect)
{
    /* for from-space, want to use full width of memory space */

    auto [x_coord_lo, x_coord_hi] = space_rect.x_span();

    // dest_space_z_ ?

    double w0 = copy_detail.dest_offset_ / static_cast<double>(copy_detail.dest_z_);
    float dest0_x = ((1.0 - w0) * x_coord_lo) + (w0 * x_coord_hi);

    double w1 = ((copy_detail.dest_offset_ + copy_detail.z_)
                 / static_cast<double>(copy_detail.dest_z_));
    float dest1_x = ((1.0 - w1) * x_coord_lo) + (w1 * x_coord_hi);

    return space_rect.with_x_span(dest0_x, dest1_x);
}

/* editor bait: animate_copy() */
void animate_gc_copy(const AppState & app_state,
                     const DrawState & draw_state,
                     ImDrawList * draw_list)
{
    /* NOTE: this only runs during GC copy.
     *       draw_state.gcw_nursery_layout_ and gcw_tenured_layout_
     *       are taken from snapshots made before GC began,
     *       ergo before to/from spaces were swapped
     */
    ImRect nursery_src_rect = draw_state.gcw_nursery_layout_.to_alloc_rect();
    ImRect tenured_src_rect = draw_state.gcw_tenured_layout_.to_alloc_rect();

    ImRect nursery_dest_rect = draw_state.gcw_nursery_layout_.mem_rect_from_;
    ImRect tenured_dest_rect;

    if (app_state.upto_ == generation::nursery) {
        tenured_dest_rect = draw_state.gcw_tenured_layout_.mem_rect_to_;
    } else {
        tenured_dest_rect = draw_state.gcw_tenured_layout_.mem_rect_from_;
    }

    std::size_t n_copy = app_state.copy_detail_v_.size();
    /* grade from black-to-white between lo_copy and hi_copy.
     * Note we allow animate_copy_hi_pct_ > 100.0
     */
    float lo_copy = 0.01 * std::max(0.0, draw_state.animate_copy_hi_pct_ - 14.0) * n_copy;
    float hi_copy = 0.01 * draw_state.animate_copy_hi_pct_ * n_copy;

    /* remember max copy offset seen in {nursery, tenured} space respectively,
     * so we can label it
     */
    std::size_t last_nursery_dest_offset = 0;
    ImRect last_nursery_dest_rect;
    std::size_t first_tenured_dest_offset = 0;
    ImRect first_tenured_dest_rect;
    std::size_t last_tenured_dest_offset = 0;
    ImRect last_tenured_dest_rect;

    std::size_t i_copy = 0;
    for (const auto & copy_detail : app_state.copy_detail_v_) {
        /* cutout for each copied object */
        {
            float wt = (i_copy > lo_copy) ? static_cast<float>(i_copy) / hi_copy : 0.0;
            /* grey fading to black */
            //ImU32 color = IM_COL32(wt*128, 64+wt*64, wt*128, 255);
            ImU32 color = IM_COL32( 96, 224, 255, 255);

            ImRect src_rect;

            if (copy_detail.src_gen_ == generation::nursery) {
                src_rect = map_src_alloc_to_screen(copy_detail, nursery_src_rect);
            } else {
                src_rect = map_src_alloc_to_screen(copy_detail, tenured_src_rect);
            }

            draw_list->AddRectFilled(src_rect.top_left(),
                                     src_rect.bottom_right(),
                                     color);
        }

        if (copy_detail.dest_gen_ == generation::nursery) {
            last_nursery_dest_rect = map_dest_alloc_to_screen(copy_detail, nursery_dest_rect);
            last_nursery_dest_offset = copy_detail.dest_offset_;
        } else if (copy_detail.dest_gen_ == generation::tenured) {
            last_tenured_dest_rect = map_dest_alloc_to_screen(copy_detail, tenured_dest_rect);
            last_tenured_dest_offset = copy_detail.dest_offset_;

            if (first_tenured_dest_rect.width() == 0) {
                first_tenured_dest_rect = last_tenured_dest_rect;
                first_tenured_dest_offset = copy_detail.dest_offset_;
            }
        }

        if (++i_copy >= hi_copy) {
            break;
        }
    }

    if (last_nursery_dest_rect.width() > 0.0) {
        //ImU32 color = IM_COL32(64, 255, static_cast<int>(64 + (128 * wt)), 255);
        ImU32 color = IM_COL32(  0, 96, 192, 255);

        draw_list->AddRectFilled(nursery_dest_rect.top_left(),
                                 last_nursery_dest_rect.bottom_right(),
                                 color);

        char buf[255];
        snprintf(buf, sizeof(buf), "N\u2081: %luk", last_nursery_dest_offset / 1024);

        auto textz = ImGui::CalcTextSize(buf);

        ImU32  text_color = IM_COL32(255, 255, 255, 255); /*black*/
        ImVec2 text_pos   = ImVec2(0.5 * (nursery_dest_rect.x_lo()
                                          + last_nursery_dest_rect.x_hi()
                                          - textz.x),
                                   nursery_dest_rect.y_mid() - 0.5 * textz.y);

        if (text_pos.x < nursery_dest_rect.x_lo())
            text_pos.x = nursery_dest_rect.x_lo() + 2;
        else if (text_pos.x < nursery_dest_rect.x_lo() + 0.5 * textz.x)
            text_pos.x = nursery_dest_rect.x_lo() + 0.5 * textz.x;

        draw_list->AddText(text_pos, text_color, buf);

    }

    if (last_tenured_dest_rect.width() > 0.0) {
        ImU32 color = IM_COL32(  0,  96, 192, 255);

        draw_list->AddRectFilled(first_tenured_dest_rect.top_left(),
                                 last_tenured_dest_rect.bottom_right(),
                                 color);

        char buf[255];
        snprintf(buf, sizeof(buf), "+%luk", (last_tenured_dest_offset - first_tenured_dest_offset) / 1024);

        auto textz = ImGui::CalcTextSize(buf);

        ImU32  text_color = IM_COL32(255, 255, 255, 255); /*black*/
        float x0 = first_tenured_dest_rect.x_lo();
        float x1 = last_tenured_dest_rect.x_hi();
        ImVec2 text_pos   = ImVec2(0.5 * (x0 + x1 - textz.x),
                                   tenured_dest_rect.y_mid() - 0.5 * textz.y);

        if (text_pos.x < x0 + 2)
            text_pos.x = x0 + 2;
        else if (text_pos.x < x0 + 0.5 * textz.x)
            text_pos.x = x0 + 0.5 * textz.x;

        draw_list->AddText(text_pos, text_color, buf);
    }
}

void
AnimateGcCopyCb::notify_gc_copy(std::size_t z,
                                const void * src_addr,
                                const void * dest_addr,
                                generation src_gen,
                                generation dest_gen)
{
    using xo::scope;
    using xo::xtag;
    using xo::gc::generation_result;
    using xo::gc::generation;
    using xo::gc::role;

    scope log(XO_DEBUG(false),
              xtag("z", z),
              xtag("src", src_addr),
              xtag("dest", dest_addr),
              xtag("src_gen", src_gen),
              xtag("dest_gen", dest_gen));

    auto [src_gen2, src_offset, src_alloc, src_size] = p_app_state_->gc_->fromspace_location_of(src_addr);

    if (src_gen2 == generation_result::not_found) {
        auto [lo, hi] = p_app_state_->gc_->nursery_span(role::from_space);

        log && log(xtag("N.from.lo", (void*)lo), xtag("N.from.hi", (void*)hi));

        assert(false);
    }

    generation src_valid_gen = xo::gc::valid_genresult2gen(src_gen2);

    auto [dest_gen2, dest_offset, _, dest_size] = p_app_state_->gc_->tospace_location_of(dest_addr);

    generation dest_valid_gen = xo::gc::valid_genresult2gen(dest_gen2);

    p_app_state_->copy_detail_v_.push_back(GcCopyDetail(z,
                                                        src_valid_gen, src_offset, src_alloc,
                                                        dest_valid_gen, dest_offset, dest_size));

    if (dest_valid_gen == generation::nursery) {
        p_app_state_->copy_detail_max_nursery_dest_offset_
            = std::max(p_app_state_->copy_detail_max_nursery_dest_offset_, dest_offset);
        p_app_state_->copy_detail_nursery_dest_size_
            = std::max(p_app_state_->copy_detail_nursery_dest_size_, dest_size);
    } else if (dest_valid_gen == generation::tenured) {
        p_app_state_->copy_detail_max_tenured_dest_offset_
            = std::max(p_app_state_->copy_detail_max_tenured_dest_offset_, dest_offset);
        p_app_state_->copy_detail_tenured_dest_size_
            = std::max(p_app_state_->copy_detail_tenured_dest_size_, dest_size);
    }

    /* will be animated across frames, see animate_gc_copy() */
}

int main(int, char **)
{
    using namespace std;

    scope log(XO_DEBUG(true));

    std::cout << "Hello, world!" << std::endl;

    SDL_SetHint(SDL_HINT_VIDEO_X11_FORCE_EGL, "0");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_version compiled;
    SDL_VERSION(&compiled);
    std::cerr << "SDL version: "
              << (int)compiled.major
              << "." << (int)compiled.minor
              << "." << (int)compiled.patch
              << std::endl;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if 0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    std::cerr << "SDL video driver: " << SDL_GetCurrentVideoDriver() << std::endl;

    SDL_Window * window = SDL_CreateWindow("imgui + sdl2 + opengl",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           2000,
                                           1000,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window) {
        std::cerr << "SDL_CreateWindow done" << std::endl;
    } else {
        std::cerr << "SDL_CreateWindow failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    std::cerr << "Requested OpenGL vtersion: " << major << "." << minor << std::endl;

    if (gl_context) {
        std::cerr << "SDL_GL_CreateContext done" << std::endl;
    } else {
        std::cerr << "SDL_GL_CreateContext failed: [" << SDL_GetError() << "]" << std::endl;
        return -1;
    }

    if (SDL_GL_MakeCurrent(window, gl_context) != 0) {
        std::cerr << "SDL_GL_MakeCurrent failed: [" << SDL_GetError() << "]" << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(0); // disable vsync

    GLenum glew_status = glewInit();
    if (glew_status == GLEW_OK) {
        std::cerr << "glewInit done" << std::endl;
    } else {
        std::cerr << "glewInit failed: [" << glewGetErrorString(glew_status) << std::endl;
        return -1;
    }

    const GLubyte * version = glGetString(GL_VERSION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (version) {
        std::cerr << "OpenGL version: [" << version << "]" << std::endl;
    } else {
        std::cerr << "OpenGL version not available" << std::endl;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load noto sans font from unix environment NOTO_ONFTS_PATH
    // (see xo-umbrella2/default.nix shellHook)

    const char * fonts_path = std::getenv("DEJAVU_FONTS_PATH");

    if (fonts_path) {
        const float font_size = 14.0f;
        std::string font_path = xo::tostr(fonts_path, "/truetype/DejaVuSans.ttf");

        /* check file exists */
        std::ifstream font_in(font_path);
        if (font_in.good()) {
            std::cerr << "loading font [" << font_path << "]" << std::endl;
            ImFont * font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
            if (font) {
                std::cerr << "font loaded" << std::endl;

                ImFontConfig config;
                config.MergeMode = true;

                // latin extended chars
                static const ImWchar latin_ranges[] = {
                    0x0020, 0x00ff,  // basic latin + latin supplement
                    0x0100, 0x017f,  // latin extended-A
                    0x0180, 0x024f,  // latin extended-B
                    0x2080, 0x2099,  // subscript numerals + letters through n
                    0x25b2, 0x25b4,  // arrows
                    0,
                };
                io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size, &config, latin_ranges);
            } else {
                std::cerr << "font file load failed" << std::endl;
                std::cerr << "Fallback to default ImGui font" << std::endl;
            }
        }


    } else {
        std::cerr << "Expected DEJAVU_FONTS_PATH environment var." << std::endl;
        std::cerr << "Fallback to default ImGui font" << std::endl;
    }
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float counter_value = 0.0f;

    using xo::obj::Integer;
    using xo::obj::List;
    using xo::rng::xoshiro256ss;
    using xo::rng::Seed;
    using xo::up;
    using xo::gp;
    using xo::gc::GC;
    using xo::Object;

    AppState app_state;
    DrawState draw_state;
    /* note: during gc copy animation, this records state _before_ gc was triggered */
    GcStateDescription gcstate = app_state.snapshot_gc_state();

    app_state.gc_->add_gc_copy_callback(draw_state.make_gc_copy_animation(&app_state));

    // Main Loop
    bool done = false;

    while (!done) {
        /** on each draw cycle, app state falls into categories:
         *  1. allocation
         *     multiple draw cycles because many allocations per gc.
         *  2. garbage collection
         *     multiple draw cycles to animate copying process
         *     Settle conflict between {GC, imgui} as to who drives event loop,
         *     in favor of imgui; achieve this by copying what GC did,
         *     so that we can animate it over multiple draw cycles
         **/

        switch (draw_state.state_type_) {
        case draw_state_type::alloc:
        {
            /** generate random alloc **/
            app_state.generate_random_mutations();

            gcstate = app_state.snapshot_gc_state();

            app_state.upto_ = (app_state.gc_->is_full_gc_pending()
                               ? generation::tenured
                               : generation::nursery);

            /* GC may run here, in which case control reenters via AnimateGcCopyCb;
             * that callback captures copy details (per object!) in AppState
             */
            if (app_state.gc_->enable_gc_once()) {
                log && log(xtag("gc-type", (app_state.upto_ == generation::tenured) ? "full" : "incremental"));

                draw_state.state_type_ = draw_state_type::animate_gc;
                draw_state.animate_copy_t0_ = std::chrono::steady_clock::now();
            }

            break;
        }
        case draw_state_type::animate_gc:
        {
            /* don't update gcstate while animating,
             * that would use post-GC space sizing
             */

            break;
        }
        }

        /** poll + handle events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    if (event.window.windowID == SDL_GetWindowID(window))
                    {
                        done = true;
                    }
                } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // handle resize immediately
                    int w, h;
                    SDL_GetWindowSize(window, &w, &h);
                    glViewport(0, 0, w, h);

                    break;  // to force render during resize
                }
            }
        }

        //int w, h;
        //SDL_GetWindowSize(window, &w, &h);
        //glViewport(0, 0, w, h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw dear imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Background", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                     | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDecoration);
        ImGui::End();

        // 1. big demo window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. show window that we create ourselves
        {
            static int counter = 0;

            ImGui::Begin("Hello, world!");
            //ImGui::Text("This is totes useful text...");
            ImGui::Checkbox("demo window", &show_demo_window);
            //ImGui::Checkbox("second window", &show_another_window);

            ImGui::SliderInt("alloc/cycle", &app_state.alloc_per_cycle_, 1, 100);
            ImGui::SliderInt("copy animation budget", &draw_state.animate_copy_budget_ms_, 10, 10000);
            //ImGui::SliderFloat("alloc/cycle", &alloc_per_cycle, 0.0f, 1.0f);
            //ImGui::ColorEdit3("clear color", (float*)&clear_color);

            //if (ImGui::Button("Button"))
            //  ++counter;
            ImGui::NewLine(); // ImGui::SameLine()
            /* N\u2080 = N0, N\u2081 = N1 */
            ImGui::Text("alloc [%lu] avail [%lu] ",
                        gcstate.gc_allocated_,
                        gcstate.gc_available_);
            //ImGui::NewLine();
            ImGui::Text("promoted [%lu] copy animation [%lu / %lu]",
                        gcstate.total_promoted_,
                        static_cast<std::size_t>(draw_state.animate_copy_hi_pct_ * app_state.copy_detail_v_.size() / 100),
                        app_state.copy_detail_v_.size());

            ImGui::Text("mutation [%lu] mlog [%lu]",
                        gcstate.total_n_mutation_,
                        gcstate.gc_mlog_size_);

            ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                        1000.0f / io.Framerate, io.Framerate);

            ImGui::Text("layout:"
                        " nursery-src alloc rect [%.1f %.1f %.1f %.1f]"
                        " nursery-dest alloc rect [%.1f %.1f %.1f %.1f]"
                        " history rect [%.1f %.1f %.1f %.1f]",
                        draw_state.gcw_nursery_layout_.to_alloc_rect().x_lo(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().y_lo(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().x_hi(),
                        draw_state.gcw_nursery_layout_.to_alloc_rect().y_hi(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().x_lo(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().y_lo(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().x_hi(),
                        draw_state.gcw_nursery_layout_.from_alloc_rect().y_hi(),
                        draw_state.gcw_history_rect_.x_lo(),
                        draw_state.gcw_history_rect_.y_lo(),
                        draw_state.gcw_history_rect_.x_hi(),
                        draw_state.gcw_history_rect_.y_hi());
            ImGui::Text("nursery-dest copy offset [%lu] / size [%lu]"
                        " tenured-dest copy offset [%lu] / size [%lu]",
                        app_state.copy_detail_max_nursery_dest_offset_,
                        app_state.copy_detail_nursery_dest_size_,
                        app_state.copy_detail_max_tenured_dest_offset_,
                        app_state.copy_detail_tenured_dest_size_
                );

            ImDrawList * draw_list = ImGui::GetWindowDrawList();

            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            /* stash so GC copy animation can find it */
            draw_state.gcw_draw_list_ = draw_list;
            draw_state.gcw_canvas_p0_ = canvas_p0;
            draw_state.gcw_canvas_p1_ = canvas_p1;

            draw_gc_state(app_state,
                          gcstate,
                          ImRect(canvas_p0, canvas_p1),
                          draw_list,
                          &draw_state.gcw_nursery_layout_,
                          &draw_state.gcw_tenured_layout_,
                          //nullptr,
                          //&draw_state.gcw_tenured_alloc_rect_,
                          &draw_state.gcw_history_rect_);

            if (draw_state.state_type_ == draw_state_type::animate_gc) {
                auto animate_copy_t1 = std::chrono::steady_clock::now();
                auto animate_dt = animate_copy_t1 - draw_state.animate_copy_t0_;
                float animate_fraction_spent
                    = (std::chrono::duration_cast<std::chrono::milliseconds>(animate_dt).count()
                       / static_cast<float>(draw_state.animate_copy_budget_ms_));

                draw_state.animate_copy_hi_pct_ = 100.0 * animate_fraction_spent;
                animate_gc_copy(app_state,
                                draw_state,
                                draw_list);

                /* see 25.0 constant in animate_gc_copy() */
                if (draw_state.animate_copy_hi_pct_ >= 114) {
                    draw_state.state_type_ = draw_state_type::alloc;
                    draw_state.animate_copy_hi_pct_ = 0;
                    app_state.copy_detail_v_.clear();
                    app_state.copy_detail_max_nursery_dest_offset_ = 0;
                    app_state.copy_detail_nursery_dest_size_ = 0;
                    app_state.copy_detail_max_tenured_dest_offset_ = 0;
                    app_state.copy_detail_tenured_dest_size_ = 0;
                }
            }

            ImGui::End();
        }

        // 3. another window
        if (show_another_window) {
            ImGui::Begin("another window", &show_another_window);

            ImGui::Text("hello from second window");
            if (ImGui::Button("close me"))
                show_another_window = false;

            ImGui::End();
        }

        // rendering
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    std::cerr << "cleanup.." << std::endl;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cerr << "All done, goodbye..." << std::endl;

    return 0;
}

/* imgui_ex4a.cpp */
#endif //DEBUG

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <iostream>
#include <vector>
#include <stdexcept>

class MinimalImGuiVulkan {
public:
    void run() {
        initWindow();
        initVulkan();
        initImGui();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        this->createSwapchain();
        this->createImageViews();
        this->createRenderPass();   // must come before createFrameBuffers
        this->createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();
    }

    void initWindow() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("Failed to initialize SDL!");
        }

        this->window_ = SDL_CreateWindow(
            "ImGui Vulkan SDL2 Example",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
        );

        if (!window_) {
            throw std::runtime_error("Failed to create SDL window!");
        }
    }

    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "ImGui Vulkan App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t extensionCount = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, nullptr)) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions!");
        }

        std::vector<const char*> extensions(extensionCount);
        if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, extensions.data())) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions!");
        }

#ifdef __apple__
        // Add portability extension for MoltenVK (macOS)
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;

#ifdef __apple__
        // CRITICAL: Enable portability enumeration flag for MoltenVK
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        int result = vkCreateInstance(&createInfo, nullptr, &(this->instance_));
        if (result != VK_SUCCESS) {
            printf("vkCreateInstance failed with error: %d\n", result);
            throw std::runtime_error("Failed to create instance!");
        }
    }

    void createSurface() {
        if (!SDL_Vulkan_CreateSurface(window_, instance_, &surface)) {
            throw std::runtime_error("Failed to create SDL Vulkan surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

        physicalDevice = devices[0]; // Just pick the first one for simplicity

        // Find graphics queue family
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
                if (presentSupport) {
                    graphicsQueueFamily = i;
                    break;
                }
            }
        }
    }

    void createLogicalDevice() {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = deviceExtensions;

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    }

    void createSwapchain() {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        VkSurfaceFormatKHR surfaceFormat = formats[0];
        swapchainImageFormat = surfaceFormat.format;

        int width, height;
        SDL_Vulkan_GetDrawableSize(window_, &width, &height);
        swapchainExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = swapchainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
    }

    void createImageViews() {
        swapchainImageViews.resize(swapchainImages.size());

        for (size_t i = 0; i < swapchainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapchainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views!");
            }
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    void createFramebuffers() {
        framebuffers.resize(swapchainImageViews.size());

        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            VkImageView attachments[] = { swapchainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapchainExtent.width;
            framebufferInfo.height = swapchainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = graphicsQueueFamily;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects!");
            }
        }
    }

    void createDescriptorPool() {
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void initImGui() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForVulkan(window_);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance_;
        init_info.PhysicalDevice = physicalDevice;
        init_info.Device = device;
        init_info.QueueFamily = graphicsQueueFamily;
        init_info.Queue = graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.RenderPass = renderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = static_cast<uint32_t>(swapchainImages.size());
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info);
        //ImGui_ImplVulkan_Init(&init_info, renderPass);

        // Upload Fonts
        VkCommandBuffer command_buffer = beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        endSingleTimeCommands(command_buffer);

        //ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void mainLoop() {
        SDL_Event event;

        while (!quit) {
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);

                if (event.type == SDL_QUIT) {
                    quit = true;
                }
            }

            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                                imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE, &imageIndex);

        switch (result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            recreateSwapchain();
            // deliberate earlyexit
            return;
        default:
            throw std::runtime_error("failed to acquire swapchain image!");
            break;
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

        if (framebuffer_resized_flag_)
            result = VK_ERROR_OUT_OF_DATE_KHR;

        switch (result) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            framebuffer_resized_flag_ = false;
            this->recreateSwapchain();
            break;
        default:
            throw std::runtime_error("failed to present swapchain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Create a simple ImGui window
        ImGui::Begin("Hello, Vulkan + SDL2!");
        ImGui::Text("This is a minimal ImGui + Vulkan + SDL2 example!");
        static float f = 0.0f;
        static int counter = 0;
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("Button"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void recreateSwapchain() {
        // handle window minimization: wait until window has valid size
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window_, &width, &height);
        while (width == 0 || height == 0) {
            SDL_GetWindowSize(window_, &width, &height);
            SDL_WaitEvent(nullptr);
        }

        // wait until device idle before cleaning up resources
        vkDeviceWaitIdle(device);

        // cleanup old swapchain
        this->cleanupFrameBuffers();
        this->cleanupImageViews();
        this->cleanupSwapchain();

        // create new swapchain
        this->createSwapchain();
        this->createImageViews();
        this->createFramebuffers();
    }

    void cleanupFrameBuffers() {
            for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }
            framebuffers.clear();
    }

    void cleanupImageViews() {
            for (auto imageView : swapchainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            swapchainImageViews.clear();
    }

    void cleanupSwapchain() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void cleanup() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        this->cleanupFrameBuffers();
        this->cleanupImageViews();
        this->cleanupSwapchain();

        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance_, surface, nullptr);
        vkDestroyInstance(instance_, nullptr);
        this->instance_ = nullptr;

        SDL_DestroyWindow(window_);
        this->window_ = nullptr;

        SDL_Quit();
    }

private:
    SDL_Window* window_ = nullptr;

    VkInstance instance_;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkDescriptorPool descriptorPool;

    uint32_t currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t graphicsQueueFamily = 0;
    /* true when window resize behavior detected,
     * until swapchain in consistent state.
     */
    bool framebuffer_resized_flag_ = false;
    bool quit = false;
}; /*MinimalImGuiVulkan*/

int main() {
    MinimalImGuiVulkan app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* end imgui_ex4a.cpp */
