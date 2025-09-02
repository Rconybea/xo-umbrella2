/* DrawState.cpp */

#include "DrawState.hpp"
#include "AnimateGcCopyCb.hpp"
#include "GcStatistics.hpp"
#include "xo/indentlog/scope.hpp"

using xo::gc::GcStatisticsHistory;
using xo::gc::generation;
using xo::scope;
using xo::xtag;

xo::up<xo::gc::GcCopyCallback>
DrawState::make_gc_copy_animation(AppState * app_state)
{
    return std::make_unique<AnimateGcCopyCb>(app_state, this);
}

/**
 *  @p polarity  0 -> draw from-space above to-space; 1 -> draw from-space below to-space
 *  @p p_x1    On exit *p_x1 contains x-coord of right-hand edge of rectangle
 *             depicting potential memory range
 **/
void
DrawState::draw_generation(const GenerationLayout & layout,
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

        ImRect::draw_filled_rect_with_label
            (layout.with_labels_ ? buf : nullptr,
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

        ImRect::draw_filled_rect_with_label
            (layout.with_labels_ ? buf : nullptr,
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
DrawState::draw_nursery(const GcStateDescription & gcstate,
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
DrawState::draw_tenured(const GcStateDescription & gcstate,
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

auto
DrawState::write_gc_history_tooltip(gc_history_headline headline,
                                    const GcStatisticsHistoryItem & stats)
    -> TooltipText
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
} /*write_gc_history_tooltip*/

/** stacked bar chart
 *
 *  @param gen  if @ref generation::nursery, only display nursery collections.
 *              otherwise display both
 **/
void
DrawState::draw_gc_history(const GcStateDescription & gcstate,
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

                ImRect::draw_filled_rect
                    (tt.c_str(),
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

                ImRect::draw_filled_rect
                    (tt.c_str(),
                     ImRect::from_xy_span(x_span, ImVec2(yp_lo, yp_hi)),
                     promote_color,
                     draw_list);
            }

            /* y-coordinates of survivor bar (survived 1st GC) */
            float ys_hi = yp_lo;
            float ys_lo = (ys_hi - (display_h * stats.survive_z_ / y_scale));
            {
                xo::flatstring<512> tt = write_gc_history_tooltip(gc_history_headline::survive, stats);

                ImRect::draw_filled_rect
                    (tt.c_str(),
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

                ImRect::draw_filled_rect
                    (tt.c_str(),
                     ImRect::from_xy_span(x_span, ImVec2(ygN_lo, ygN_hi)),
                     garbageN_color,
                     draw_list);
            }

            /* y-coordinates of garbage1 bar (killed on 2nd GC) */
            float yg1_lo = ygN_hi;
            float yg1_hi = (yg1_lo
                            + (display_h * stats.garbage1_z_ / y_scale));
            {
                TooltipText tt = write_gc_history_tooltip(gc_history_headline::garbage1, stats);

                ImRect::draw_filled_rect
                    (tt.c_str(),
                     ImRect(ImVec2(x_lo, yg1_lo), ImVec2(x_hi, yg1_hi)),
                     garbage1_color,
                     draw_list);
            }

            /* y-coordinates of garbage0 bar (killed on 1st GC) */
            float yg0_lo = yg1_hi;
            float yg0_hi = (yg0_lo
                            + (display_h * stats.garbage0_z_ / y_scale));
            {
                TooltipText tt = write_gc_history_tooltip(gc_history_headline::garbage0, stats);

                ImRect::draw_filled_rect
                    (tt.c_str(),
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
DrawState::draw_gc_efficiency(const GcStateDescription & gcstate,
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
DrawState::draw_gc_alloc_state(const GcStateDescription & gcstate,
                               const ImRect & canvas_rect,
                               ImDrawList * draw_list,
                               GenerationLayout * p_nursery_layout,
                               GenerationLayout * p_tenured_layout)
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
DrawState::draw_gc_state(const AppState & app_state,
                         const GcStateDescription & gcstate,
                         const ImRect & canvas_rect,
                         ImDrawList * draw_list,
                         GenerationLayout * p_nursery_layout,
                         GenerationLayout * p_tenured_layout,
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

ImRect
DrawState::map_src_alloc_to_screen(const GcCopyDetail & copy_detail,
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

ImRect
DrawState::map_dest_alloc_to_screen(const GcCopyDetail & copy_detail,
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
void
DrawState::animate_gc_copy(const AppState & app_state,
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

/* end DrawState.cpp */
