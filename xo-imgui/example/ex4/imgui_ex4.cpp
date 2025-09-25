/* imgui_ex4.cpp */

#include "xo/imgui/VulkanApp.hpp"
#ifdef TEMPORARILY_REMOVE
#include "xo/imgui/ImRect.hpp"
#endif
#include "AppState.hpp"
#include "DrawState.hpp"
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>

#include "xo/indentlog/scope.hpp"

#include <iostream>
#include <fstream>

namespace {
    using xo::gc::generation;
    using xo::scope;
    using xo::xtag;


    void
    app_duty_cycle_top(AppState * p_app_state,
                       DrawState * p_draw_state)
    {
        scope log(XO_DEBUG(false));

        log && log(xtag("imgui_cx", (void*)ImGui::GetCurrentContext()));

        /** on each draw cycle, app state falls into categories:
         *  1. allocation
         *     multiple draw cycles because many allocations per gc.
         *  2. garbage collection
         *     multiple draw cycles to animate copying process
         *     Settle conflict between {GC, imgui} as to who drives event loop,
         *     in favor of imgui; achieve this by copying what GC did,
         *     so that we can animate it over multiple draw cycles
         **/

        switch (p_draw_state->state_type_) {
        case draw_state_type::alloc:
        {
            /** generate random alloc **/
            p_app_state->generate_random_mutations();

            p_draw_state->gcstate_ = p_app_state->snapshot_gc_state();

            p_app_state->upto_ = (p_app_state->gc_->is_full_gc_pending()
                                  ? generation::tenured
                                  : generation::nursery);

            /* GC may run here, in which case control reenters via AnimateGcCopyCb;
             * that callback captures copy details (per object!) in AppState
             */
            if (p_app_state->gc_->enable_gc_once()) {
                scope log(XO_DEBUG(true));

                log && log(xtag("gc-type", (p_app_state->upto_ == generation::tenured) ? "full" : "incremental"));

                p_draw_state->state_type_ = draw_state_type::animate_gc;
                p_draw_state->animate_copy_t0_ = std::chrono::steady_clock::now();
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
    } /*app_duty_cycle_top*/

    VulkanApp::ImguiDrawFn
    make_imgui_draw_frame(AppState * p_app_state,
                          DrawState * p_draw_state,
                          float * p_f, int * p_counter)
    {
        *p_f = 0.0f;
        *p_counter = 0;

        return [p_app_state, p_draw_state, p_f, p_counter](ImGuiContext * imgui_cx)
            {
                scope log(XO_DEBUG(false));

#ifdef TEMPORARILY_REMOVE
                app_duty_cycle_top(p_app_state, p_draw_state);
#endif

                log && log(xtag("imgui_cx", (void*)ImGui::GetCurrentContext()));

                // Start the Dear ImGui frame
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

#ifdef TEMPORARILY_REMOVE
                log && log("after NewFrame", xtag("imgui_cx", (void*)ImGui::GetCurrentContext()));

                ImGuiIO & io = ImGui::GetIO(); (void)io;

# ifdef NOT_WORKING
                // background
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(io.DisplaySize);
                ImGui::Begin("Background", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                             | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDecoration);
                ImGui::End();
# endif
#endif

                // 1. create a simple ImGui window
                ImGui::Begin("Hello, Vulkan + SDL2!");
                ImGui::Text("This is a minimal ImGui + Vulkan + SDL2 example!");
#ifdef TEMPORARILY_REMOVE
                ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                            1000.0f / io.Framerate, io.Framerate);

                ImGui::Checkbox("demo window", &p_draw_state->show_demo_window_);

                ImGui::SliderInt("alloc/cycle", &p_app_state->alloc_per_cycle_, 1, 100);
                ImGui::SliderInt("copy animation budget", &p_draw_state->animate_copy_budget_ms_, 10, 10000);
                ImGui::NewLine();

                /* N\u2080 = N0, N\u2081 = N1 */
                ImGui::Text("alloc [%lu] avail [%lu] ",
                            p_draw_state->gcstate_.gc_allocated_,
                            p_draw_state->gcstate_.gc_available_);

                ImGui::Text("promoted [%lu] copy animation [%lu / %lu]",
                            p_draw_state->gcstate_.total_promoted_,
                            static_cast<std::size_t>(p_draw_state->animate_copy_hi_pct_
                                                     * p_app_state->copy_detail_v_.size() / 100),
                            p_app_state->copy_detail_v_.size());

                ImGui::Text("mutation [%lu] mlog [%lu]",
                            p_draw_state->gcstate_.total_n_mutation_,
                            p_draw_state->gcstate_.gc_mlog_size_);

                ImGui::Text("appl average %.3f ms/frame (%.1f fps)",
                            1000.0f / io.Framerate, io.Framerate);

                ImGui::Text("layout:"
                            " nursery-src alloc rect [%.1f %.1f %.1f %.1f]"
                            " nursery-dest alloc rect [%.1f %.1f %.1f %.1f]"
                            " history rect [%.1f %.1f %.1f %.1f]",
                            p_draw_state->gcw_nursery_layout_.to_alloc_rect().x_lo(),
                            p_draw_state->gcw_nursery_layout_.to_alloc_rect().y_lo(),
                            p_draw_state->gcw_nursery_layout_.to_alloc_rect().x_hi(),
                            p_draw_state->gcw_nursery_layout_.to_alloc_rect().y_hi(),
                            p_draw_state->gcw_nursery_layout_.from_alloc_rect().x_lo(),
                            p_draw_state->gcw_nursery_layout_.from_alloc_rect().y_lo(),
                            p_draw_state->gcw_nursery_layout_.from_alloc_rect().x_hi(),
                            p_draw_state->gcw_nursery_layout_.from_alloc_rect().y_hi(),
                            p_draw_state->gcw_history_rect_.x_lo(),
                            p_draw_state->gcw_history_rect_.y_lo(),
                            p_draw_state->gcw_history_rect_.x_hi(),
                            p_draw_state->gcw_history_rect_.y_hi());
                ImGui::Text("nursery-dest copy offset [%lu] / size [%lu]"
                            " tenured-dest copy offset [%lu] / size [%lu]",
                            p_app_state->copy_detail_max_nursery_dest_offset_,
                            p_app_state->copy_detail_nursery_dest_size_,
                            p_app_state->copy_detail_max_tenured_dest_offset_,
                            p_app_state->copy_detail_tenured_dest_size_);

                ImDrawList * draw_list = ImGui::GetWindowDrawList();

                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

                /* stash so GC copy animation can find it */
                p_draw_state->gcw_draw_list_ = draw_list;
                p_draw_state->gcw_canvas_p0_ = canvas_p0;
                p_draw_state->gcw_canvas_p1_ = canvas_p1;

                DrawState::draw_gc_state(*p_app_state,
                                         p_draw_state->gcstate_,
                                         ImRect(canvas_p0, canvas_p1),
                                         draw_list,
                                         &p_draw_state->gcw_nursery_layout_,
                                         &p_draw_state->gcw_tenured_layout_,
                                         &p_draw_state->gcw_history_rect_);

                if (p_draw_state->state_type_ == draw_state_type::animate_gc) {
                    auto animate_copy_t1 = std::chrono::steady_clock::now();
                    auto animate_dt = animate_copy_t1 - p_draw_state->animate_copy_t0_;
                    float animate_fraction_spent
                        = (std::chrono::duration_cast<std::chrono::milliseconds>(animate_dt).count()
                           / static_cast<float>(p_draw_state->animate_copy_budget_ms_));

                    p_draw_state->animate_copy_hi_pct_ = 100.0 * animate_fraction_spent;

                    DrawState::animate_gc_copy(*p_app_state,
                                               *p_draw_state,
                                               draw_list);

                    /* see 25.0 constant in animate_gc_copy() */
                    if (p_draw_state->animate_copy_hi_pct_ >= 114) {
                        p_draw_state->state_type_                         = draw_state_type::alloc;
                        p_draw_state->animate_copy_hi_pct_                = 0;
                        p_app_state->copy_detail_v_.clear();
                        p_app_state->copy_detail_max_nursery_dest_offset_ = 0;
                        p_app_state->copy_detail_nursery_dest_size_       = 0;
                        p_app_state->copy_detail_max_tenured_dest_offset_ = 0;
                        p_app_state->copy_detail_tenured_dest_size_       = 0;
                    }
                }
#endif

                ImGui::End();

#ifdef TEMPORARILY_REMOVE
                // 2. big demo window
                if (p_draw_state->show_demo_window_)
                    ImGui::ShowDemoWindow(&p_draw_state->show_demo_window_);
#endif

                // Rendering
                ImGui::Render();
                return ImGui::GetDrawData();
            };
    }

    void app_imgui_load_fonts(ImGuiContext * imgui_cx)
    {
        scope log(XO_DEBUG(false));
        log && log(xtag("imgui_cx", (void*)ImGui::GetCurrentContext()));

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
    } /*app_imgui_load_fonts*/
}

int main() {
    printf("Hello world\n");

    scope log(XO_DEBUG(true));
    log && log("starting main");

    AppState app_state;
    DrawState draw_state;
    draw_state.gcstate_ = app_state.snapshot_gc_state();

    float f = 0.0;
    int counter = 0;
    VulkanApp::ImguiDrawFn draw_fn
        = make_imgui_draw_frame(&app_state, &draw_state, &f, &counter);
    VulkanApp vk_app(draw_fn);

    /* establishes imgui context */
    vk_app.setup(app_imgui_load_fonts);

#ifdef NOT_YET
    app_state.gc_->add_gc_copy_callback
        (draw_state.make_gc_copy_animation(&app_state));
#endif

    try {
        vk_app.main_loop();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    vk_app.cleanup();

    return EXIT_SUCCESS;
}

/* end imgui_ex4.cpp */
