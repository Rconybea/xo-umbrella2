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
#include <unistd.h>

using xo::gc::generation;

struct ImRect {
    ImRect() = default;
    ImRect(const ImVec2 & tl, const ImVec2 & br) : top_left_{tl}, bottom_right_{br} {}

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

    ImVec2 top_left_{0, 0};
    ImVec2 bottom_right_{0, 0};
};

/* We need GUI to be able to fall behind true GC state, so we can animate transitions.
 * To help make this work, provide a model for GC state sufficient to drive rendering.
 */
struct GcGenerationDescription {
    GcGenerationDescription() = default;
    GcGenerationDescription(const char * mnemonic, std::size_t before_ckp, std::size_t after_ckp,
                            std::size_t reserved, std::size_t committed, std::size_t gc_threshold)
        : mnemonic_{mnemonic}, before_checkpoint_{before_ckp}, after_checkpoint_{after_ckp},
          reserved_{reserved}, committed_{committed},
          gc_threshold_{gc_threshold} {}

    /** scale (in bytes) for drawing space **/
    std::size_t scale() const { return std::max(committed_, gc_threshold_); }

    const char * mnemonic_ = nullptr;
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

struct AppState {
    using GC = xo::gc::GC;

public:
    AppState();

    std::size_t nursery_tospace_scale() const;
    std::size_t tenured_tospace_scale() const;
    GcStateDescription snapshot_gc_state() const;

    void generate_random_mutation();

public:
    up<GC> gc_;
    std::size_t next_int_ = 0;
    std::size_t next_root_ = 0;
    std::vector<gp<Object>> gc_root_v_{100};
    Seed<xoshiro256ss> seed_;
    xoshiro256ss rng_{seed_};
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
    return GcStateDescription(GcGenerationDescription
                              ("N",
                               gc_->nursery_before_checkpoint(),
                               gc_->nursery_after_checkpoint(),
                               gc_->nursery_to_reserved(),
                               gc_->nursery_to_committed(),
                               gc_->nursery_before_checkpoint() + gc_->config().incr_gc_threshold_),
                              GcGenerationDescription
                              ("T",
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
draw_filled_rect_with_label(const char * text,
                            const char * tooltip,
                            const ImRect & rect,
                            ImU32 fillcolor,
                            ImU32 textcolor,
                            ImDrawList * draw_list)
{
    draw_list->AddRectFilled(rect.top_left(),
                             rect.bottom_right(),
                             fillcolor); //IM_COL32(0, 128, 0, 255) /*darker green*/);

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

using xo::scope;

/**
 *  @p p_alloc_p0 @p p_alloc_p1 On exit contains corners of rectangle
 *             depicting allocated memory range
 *  @p p_x1    On exit *p_x1 contains x-coord of right-hand edge of rectangle
 *             depicting potential memory range
 **/
void
draw_generation(const GcStateDescription & gcdescr,
                xo::gc::generation gen,
                bool with_labels,
                const ImRect & bounding_rect,
                ImDrawList * draw_list,
                ImRect * p_alloc_rect,
                float * p_x1)
{
    //scope log(XO_DEBUG(with_labels));

    using xo::gc::generation;

    /* mnemonic for gneeration.  'N' <-> nursery, 'T' <-> tenured */
    const char * G_mnemonic = "";
    /* bytes allocated to this generation since last GC
     * if nursery: new allocation
     * if tenured: promotions since last full GC
     */
    std::size_t G0_to_size = 0;
    /* bytes used for residents of this generation that have survived at least one GC */
    std::size_t G1_to_size = 0;
    /* bytes of reserved memory for this generation's to-space */
    std::size_t G_to_reserved = 0;
    /* bytes of committed memory for this generation's to-space */
    std::size_t G_to_committed = 0;
    /* next GC trigges when G0_to_size reaches this threshold */
    std::size_t G_to_gc_threshold = 0;

    const GcGenerationDescription & gendescr = gcdescr.gen_state_v_[gen2int(gen)];

    G_mnemonic = gendescr.mnemonic_;
    G1_to_size = gendescr.before_checkpoint_;
    G0_to_size = gendescr.after_checkpoint_;
    G_to_reserved = gendescr.reserved_;
    G_to_committed = gendescr.committed_;
    G_to_gc_threshold = gendescr.gc_threshold_;

    std::size_t G_to_scale = gendescr.scale();

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

    ImRect chart_rect = bounding_rect;
    /* e.g. N1: 34511 bytes */
    char g1_buf[255];

    if (with_labels) {
        snprintf(g1_buf, sizeof(g1_buf), "reserved: %lu bytes; committed: %lu bytes; %s\u2081: %lu bytes; %s\u2080: %lu bytes",
                 G_to_reserved, G_to_committed, G_mnemonic, G1_to_size, G_mnemonic, G0_to_size);

        auto textz = ImGui::CalcTextSize(g1_buf);

        assert(textz.y < bounding_rect.height());

        chart_rect = bounding_rect.with_y_span(bounding_rect.y_lo() + textz.y + 2, bounding_rect.y_hi());

        draw_list->AddText(bounding_rect.top_left(),
                           IM_COL32(255, 255, 192, 255),
                           g1_buf);
    }

    float rh_text_dx = 0.0;

    /* rhs label text, e.g "Mem: 36k" */
    if (with_labels) {
        char buf[255];
        snprintf(buf, sizeof(buf), "%s: %luk",
                 (G_to_gc_threshold > G_to_committed) ? G_mnemonic : "Mem",
                 G_to_scale / 1024);

        auto textz = ImGui::CalcTextSize(buf);

        rh_text_dx = 5 + textz.x;

        draw_list->AddText(ImVec2(chart_rect.x_hi() - textz.x,
                                  chart_rect.y_mid() - 0.5 * textz.y),
                           IM_COL32(255, 255, 255, 255),
                           buf);
    }

    /* chart rectangle */
    // TODO: rect.with_x_span(rect.x_lo(), rect.x_hi() - rh_text_dx)
    draw_list->AddRect(chart_rect.top_left(),
                       ImVec2(chart_rect.x_hi() - rh_text_dx, chart_rect.y_hi()),
                       IM_COL32(255, 255, 255, 255) /*white*/);

    float display_w = chart_rect.width() - rh_text_dx;
    float G1_w = (display_w * G1_to_size) / G_to_scale;
    // TODO: rect.with_x_span(rect.x_lo(), rect.x_lo() + G1_w)
    float G1_x1 = chart_rect.x_lo() + G1_w;
    ImVec2 G1_p1(G1_x1, chart_rect.y_hi());

    /* G1 */
    {

        char buf[255];

        if (with_labels)
            snprintf(buf, sizeof(buf), "%s\u2081: %luk", G_mnemonic, G1_to_size / 1024); /* N(1) */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2081: %lu - %s survivor size in bytes",
                 G_mnemonic, G1_to_size,
                 ((gen == xo::gc::generation::nursery) ? "nursery" : "tenured"));

        draw_filled_rect_with_label(with_labels ? buf : nullptr,
                                    tooltip,
                                    // TODO: rect.with_x_span(rect.x_lo(), rect.x_lo() + G1_w)
                                    ImRect(chart_rect.top_left(), G1_p1),
                                    IM_COL32(  0, 128,   0, 255) /*darker green*/,
                                    IM_COL32(255, 255, 255, 255) /*white*/,
                                    draw_list);
    }

    float G0_x0 = G1_x1;
    float G0_x1 = G0_x0 + (display_w * G0_to_size) / G_to_scale;
    // TODO: rect.with_x_span(G0_x0, G0_x1);
    ImVec2 G0_p0(G0_x0, chart_rect.y_lo());
    ImVec2 G0_p1(G0_x1, chart_rect.y_hi());

    /* G0 */
    {

        char buf[255];

        if (with_labels)
            snprintf(buf, sizeof(buf), "%s\u2080: %luk", G_mnemonic, G0_to_size / 1024); /* N(0) */

        char tooltip[255];
        snprintf(tooltip, sizeof(tooltip),
                 "%s\u2080: %lu - %s new alloc size in bytes",
                 G_mnemonic, G0_to_size,
                 ((gen == xo::gc::generation::nursery) ? "nursery" : "tenured"));

        draw_filled_rect_with_label(with_labels ? buf : nullptr,
                                    tooltip,
                                    ImRect(G0_p0, G0_p1),
                                    IM_COL32( 32, 192,  32, 255) /*lighter green*/,
                                    IM_COL32(  0,   0,   0, 255) /*black*/,
                                    draw_list);
    }

    /* mark where next gc will trigger */
    if (with_labels) {
        const char * uparrow = reinterpret_cast<const char *>(u8"\u25b3");

        float ngc_w = (display_w * G_to_gc_threshold) / G_to_scale;

        auto tmp = ImGui::CalcTextSize(uparrow);
        std::size_t uparrow_w = tmp.x;
        double ngc_x = chart_rect.x_lo() + ngc_w - uparrow_w/2.0;

        ImVec2 marker_pos(ngc_x, chart_rect.y_hi());

        draw_list->AddText(marker_pos,
                           IM_COL32(255, 128, 128, 255) /*red*/,
                           uparrow);

        ImGui::SetCursorScreenPos(marker_pos);
        ImGui::InvisibleButton("mkbutton", tmp);
        if (ImGui::IsItemHovered()) {
            char marker_tt_buf[255];
            snprintf(marker_tt_buf, sizeof(marker_tt_buf),
                     "Next %s GC when size(%s) >= %lu bytes",
                     (gen == generation::nursery ? "incremental" : "full"),
                     (gen == generation::nursery ? "nursery" : "tenured"),
                     G_to_gc_threshold);

            ImGui::SetTooltip("%s", marker_tt_buf);
        }
    }

    if (p_alloc_rect)
        *p_alloc_rect = chart_rect.with_x_span(chart_rect.x_lo(), G0_x1);
    if (p_x1)
        *p_x1 = chart_rect.x_hi() - rh_text_dx;
}

void
draw_nursery(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             ImRect * p_alloc_rect,
             float * p_x1)
{
    using xo::gc::generation;

    draw_generation(gcstate,
                    generation::nursery,
                    with_labels,
                    rect,
                    draw_list,
                    p_alloc_rect,
                    p_x1);
}

void
draw_tenured(const GcStateDescription & gcstate,
             bool with_labels,
             const ImRect & rect,
             ImDrawList * draw_list,
             ImRect * p_alloc_rect,
             float * p_x1)
{
    using xo::gc::generation;

    draw_generation(gcstate,
                    generation::tenured,
                    with_labels,
                    rect,
                    draw_list,
                    p_alloc_rect,
                    p_x1);
}

using xo::gc::GC;
using xo::gc::GcStatisticsExt;
using xo::gc::GcStatisticsHistory;
using xo::gc::GcStatisticsHistoryItem;
using xo::xtag;
using std::size_t;

void
draw_gc_history(const GcStateDescription & gcstate,
                generation gen,
                const GcStatisticsHistory & gc_history,
                const ImRect & bounding_rect,
                bool debug_flag,
                ImDrawList * draw_list)
{
    scope log(XO_DEBUG(debug_flag));

    float lm = 50;
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
    }

    /* y-coord of x-axis */
    float y_zero = bounding_rect.y_lo() + tm + (display_h * yplus_scale) / (yplus_scale + yminus_scale);

    float y_scale = yplus_scale + yminus_scale;

    /* width of 1 bar in screen coords */
    float bar_w = display_w / gc_history.capacity();

    /* 2nd loop: draw bars */
    std::size_t i = 0;
    for (const GcStatisticsHistoryItem & stats : gc_history) {
        /* x-coordinates of bar */
        float x_lo = lm + i * bar_w;
        float x_hi = x_lo + bar_w - 1;

        /* y-coordinates of persist bar (survived 3+ GCs) */
        float ypsz_lo = (y_zero
                         - (display_h * stats.persist_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, ypsz_lo), ImVec2(x_hi, y_zero),
                                  IM_COL32(  0,  64, 192, 255) /*darker blue*/);

        /* y-coordinates of promote bar (survived 2nd GC) */
        float yp_hi = ypsz_lo;
        float yp_lo = (yp_hi
                      - (display_h * stats.promote_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, yp_lo), ImVec2(x_hi, yp_hi),
                                 IM_COL32(  0, 128,   0, 255) /*darker green*/);

        /* y-coordinates of survivor bar (survived 1st GC) */
        float ys_hi = yp_lo;
        float ys_lo = (ys_hi - (display_h * stats.survive_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, ys_lo), ImVec2(x_hi, ys_hi),
                                 IM_COL32( 32, 192,  32, 255));

        // -----------------------------------------------------------

        /* y-coordinates of garbageN bar (killed on 3+ GC) */
        float ygN_lo = y_zero;
        float ygN_hi = (y_zero
                        + (display_h * stats.garbageN_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, ygN_lo), ImVec2(x_hi, ygN_hi),
                                 IM_COL32(255, 192,  32, 255));

        /* y-coordinates of garbage1 bar (killed on 2nd GC) */
        float yg1_lo = ygN_hi;
        float yg1_hi = (yg1_lo
                        + (display_h * stats.garbage1_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, y_zero), ImVec2(x_hi, yg1_hi),
                                 IM_COL32(192, 192,  32, 255));

        /* y-coordinates of garbage0 bar (killed on 1st GC) */
        float yg0_lo = yg1_hi;
        float yg0_hi = (yg0_hi
                        + (display_h * stats.garbage0_z_ / y_scale));

        draw_list->AddRectFilled(ImVec2(x_lo, yg0_lo), ImVec2(x_hi, yg0_hi),
                                 IM_COL32(255, 255,  32, 255));

        ++i;
    }

    log && log(xtag("i", i));
}

void
draw_gc_state(const AppState & app_state,
              const GcStateDescription & gcstate,
              const ImRect & canvas_rect,
              ImDrawList * draw_list,
              ImRect * p_nursery_alloc_rect)
{
    float lm = 50;
    float rm = 70;
    float tm = 10;
    float est_chart_text_height = 14;
    float h = 20;  // chart bar height

    // draw stuff
    draw_list->AddRect(canvas_rect.top_left(),
                       canvas_rect.bottom_right(),
                       IM_COL32(255, 255, 255, 255));

    /* bounding rectange for nursery display */
    ImRect N_space_rect(ImVec2(canvas_rect.x_lo() + lm,
                               canvas_rect.y_lo() + tm),
                        ImVec2(canvas_rect.x_hi() - rm,
                               canvas_rect.y_lo() + tm + h + est_chart_text_height));
    /* rectangle representing allocated nursery range*/
    ImRect N_alloc_rect;
    float N_x1 = 0.0;

    draw_nursery(gcstate,
                 true /*with_labels*/,
                 N_space_rect,
                 draw_list,
                 &N_alloc_rect,
                 &N_x1);

    if (p_nursery_alloc_rect)
        *p_nursery_alloc_rect = N_alloc_rect;

    /* N0_to_size..N_to_scale: in bytes */
    std::size_t N_to_scale = app_state.nursery_tospace_scale();

    /* display_w .. N0_h : viewportcoords */
    std::size_t display_w = canvas_rect.width() - lm - rm;

    std::size_t x0 = canvas_rect.x_lo() + lm;
    std::size_t x1 = canvas_rect.x_hi() - rm;
    std::size_t n_y0 = canvas_rect.y_lo() + tm;
    std::size_t n_y1 = n_y0 + h;

    // now turn to Tenured space

    std::size_t T_to_scale = app_state.tenured_tospace_scale();
    std::size_t T1_h = h;

    /* want to put to-scale image of nursery next to to-scale image of tenured;
     * but also want space between them.
     */
    float TplusN_to_scale = N_to_scale + T_to_scale;
    /* space between T, N images */
    float TplusN_spacer = 10;

    /* for side-by-side tenured + nursery, with both on same scale
     * 2nd term is horiz space used for N label like 'Mem: 28k'
     */
    std::size_t adj_display_w = display_w - (N_space_rect.x_hi() - N_x1);

    /* for smaller image of nursery */
    std::size_t t_y0 = canvas_rect.y_lo() + 70 + h + 20;
    std::size_t t_y1 = t_y0 + T1_h;

    /* bounding rectangle for secondary nursery display */
    ImRect np_rect(ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)),
                          t_y0 + est_chart_text_height),
                   ImVec2(x0 + adj_display_w,
                          t_y1 + est_chart_text_height));

    // redraw nursery to same scale as tenured
    {
        draw_list->AddLine(N_space_rect.bottom_left(), np_rect.top_left(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);
        draw_list->AddLine(ImVec2(N_x1, N_space_rect.y_hi()), np_rect.top_right(),
                           IM_COL32(128, 128, 128, 255) /*grey*/);

        draw_nursery(gcstate,
                     false /*no labels*/,
                     np_rect, //ImRect(ImVec2(np_x0, np_y0), ImVec2(np_x1, np_y1)),
                     draw_list,
                     nullptr,
                     nullptr);
    }

    std::size_t h_y0 = t_y1 + est_chart_text_height;

    draw_tenured(gcstate,
                 true /*with labels*/,
                 ImRect(ImVec2(x0, t_y0),
                        ImVec2(x0 + (adj_display_w * (T_to_scale/TplusN_to_scale)) - TplusN_spacer,
                               h_y0)),
                 draw_list,
                 nullptr,
                 nullptr);

    draw_gc_history(gcstate,
                    generation::nursery,
                    app_state.gc_->gc_history(),
                    ImRect(ImVec2(x0, h_y0),
                           ImVec2(x1, h_y0 + 250)),
                    false /*debug_flag*/,
                    draw_list);

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

struct DrawState {
    up<xo::gc::GcCopyCallback> make_gc_copy_animation(AppState * app_state) {
        return std::make_unique<AnimateGcCopyCb>(app_state, this);
    }

    ImDrawList * gcw_draw_list_ = nullptr;

    /* draw area */
    ImVec2 gcw_canvas_p0_;
    ImVec2 gcw_canvas_p1_;

    /** rect displaying allocated nursery space **/
    ImRect gcw_nursery_alloc_rect_;
};

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
    using xo::gc::role;

    scope log(XO_DEBUG(false),
              xtag("z", z),
              xtag("src", src_addr),
              xtag("dest", dest_addr),
              xtag("src_gen", src_gen),
              xtag("dest_gen", dest_gen));

    const ImRect & nursery_rect = p_draw_state_->gcw_nursery_alloc_rect_;

    auto [x_coord_lo, x_coord_hi] = nursery_rect.x_span();

    auto [gen, offset, alloc] = p_app_state_->gc_->fromspace_location_of(src_addr);

    if (gen == generation_result::not_found) {
        auto [lo, hi] = p_app_state_->gc_->nursery_span(role::from_space);

        log && log(xtag("N.from.lo", (void*)lo), xtag("N.from.hi", (void*)hi));

        assert(false);
    }

    double w0 = offset / static_cast<double>(alloc);
    float src0_x = ((1.0 - w0) * x_coord_lo) + (w0 * x_coord_hi);

    double w1 = (offset + z) / static_cast<double>(alloc);
    float src1_x = ((1.0 - w1) * x_coord_lo) + (w1 * x_coord_hi);

    ImRect src_rect = nursery_rect.with_x_span(src0_x, src1_x);

    p_draw_state_->gcw_draw_list_->AddRectFilled(src_rect.top_left(),
                                                 src_rect.bottom_right(),
                                                 IM_COL32(255, 128, 128, 255));
}

int main(int, char **)
{
    using namespace std;

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

    std::cerr << "Requested OpenGL version: " << major << "." << minor << std::endl;

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

    SDL_GL_SetSwapInterval(1); // enable vsync

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
                    0x2080, 0x208a,  // subscript numerals
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

    app_state.gc_->add_gc_copy_callback(draw_state.make_gc_copy_animation(&app_state));

    // Main Loop
    bool done = false;

    while (!done) {
        /** generate random alloc **/
        app_state.generate_random_mutation();

        GcStateDescription gcstate = app_state.snapshot_gc_state();

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
            ImGui::Text("This is totes useful text...");
            ImGui::Checkbox("demo window", &show_demo_window);
            ImGui::Checkbox("second window", &show_another_window);

            ImGui::SliderFloat("float", &counter_value, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);

            if (ImGui::Button("Button"))
                ++counter;
            ImGui::NewLine(); // ImGui::SameLine()
            /* \u2080 = N0, \u2081 = N1 */
            ImGui::Text("alloc [%lu] avail [%lu] ",
                        gcstate.gc_allocated_,
                        gcstate.gc_available_);
            //ImGui::NewLine();
            ImGui::Text("promoted [%lu]",
                        gcstate.total_promoted_);

            ImGui::Text("mutation [%lu] mlog [%lu]",
                        gcstate.total_n_mutation_,
                        gcstate.gc_mlog_size_);

            ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                        1000.0f / io.Framerate, io.Framerate);

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
                          &draw_state.gcw_nursery_alloc_rect_);

            app_state.gc_->enable_gc();
            /* GC may run here, in which case control reenters via AnimateGcCopyCb;
             * callback will rely on loop assignments to draw_area members.
             */
            app_state.gc_->disable_gc();

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

/* imgui_ex2.cpp */
