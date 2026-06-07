/* DrawState.hpp */

#pragma once

#include "GcStatistics.hpp"
#include "xo/alloc/GC.hpp"
#include "GenerationLayout.hpp"
#include "GcStateDescription.hpp"
#include "GcCopyDetail.hpp"
#include "imgui.h"
#include <chrono>

struct AppState;

enum class draw_state_type {
    alloc,
    animate_gc
};

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

struct DrawState {
    using GcStatisticsHistory = xo::gc::GcStatisticsHistory;
    using generation = xo::gc::generation;

    using TooltipText = xo::flatstring<512>;
    using GcStatisticsHistoryItem = xo::gc::GcStatisticsHistoryItem;

    static void draw_generation(const GenerationLayout & layout,
                                ImDrawList * draw_list);
    static void draw_nursery(const GcStateDescription & gcstate,
                             bool with_labels,
                             const ImRect & rect,
                             ImDrawList * draw_list,
                             GenerationLayout * p_layout);
    static void draw_tenured(const GcStateDescription & gcstate,
                             bool with_labels,
                             const ImRect & rect,
                             ImDrawList * draw_list,
                             GenerationLayout * p_layout);
    static ImU32 headline_color(gc_history_headline headline);

    /**
     * display colored bullet (at current cursor position) for a text item
     **/
    static void draw_text_bullet(ImU32 fill_color,
                                 ImDrawList * draw_list);
    /**
     * display one bar in bar strip chart displaying gc statistics
     *
     * @p idname : name for invisible button widget (expected to be unique)
     * @p headline : purpose of rectangle
     * @p stats : gc statistics for this bar.
     * @p bar_rect : rectangle representing quantity in bytes
     * @p draw_list : draw list for current frame
     **/
    static void write_gc_history_bar(const char * idname,
                                     gc_history_headline headline,
                                     const GcStatisticsHistoryItem & stats,
                                     const ImRect & bar_rect,
                                     ImDrawList * draw_list);
    static void draw_gc_history(const GcStateDescription & gcstate,
                                generation gen,
                                const GcStatisticsHistory & gc_history,
                                const ImRect & bounding_rect,
                                bool debug_flag,
                                ImDrawList * draw_list);
    static void draw_gc_efficiency(const GcStateDescription & gcstate,
                                   const GcStatisticsHistory & gc_history,
                                   const ImRect & bounding_rect,
                                   bool debug_flag,
                                   ImDrawList * draw_list);
    static void draw_gc_alloc_state(const GcStateDescription & gcstate,
                                    const ImRect & canvas_rect,
                                    ImDrawList * draw_list,
                                    GenerationLayout * p_nursery_layout,
                                    GenerationLayout * p_tenured_layout);
    static void draw_gc_state(const AppState & app_state,
                              const GcStateDescription & gcstate,
                              const ImRect & canvas_rect,
                              ImDrawList * draw_list,
                              GenerationLayout * p_nursery_layout,
                              GenerationLayout * p_tenured_layout,
                              ImRect * p_history_rect);
    xo::up<xo::gc::GcCopyCallback> make_gc_copy_animation(AppState * app_state);
    static ImRect map_src_alloc_to_screen(const GcCopyDetail & copy_detail,
                                          const ImRect & space_rect);
    static ImRect map_dest_alloc_to_screen(const GcCopyDetail & copy_detail,
                                           const ImRect & space_rect);

    static void animate_gc_copy(const AppState & app_state,
                                const DrawState & draw_state,
                                ImDrawList * draw_list);

public:
    /** when true display imgui demo window **/
    bool show_demo_window_ = false;
    /** whether vsync feature enabled (throttle to ~60 fps) **/
    bool vsync_enabled_ = true;

    draw_state_type state_type_ = draw_state_type::alloc;

    /** note: during gc copy animation,
     *       this records state _before_ gc was triggered
     **/
    GcStateDescription gcstate_;
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

/* end DrawState.hpp */
