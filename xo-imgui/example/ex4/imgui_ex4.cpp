/* imgui_ex4.cpp */

#include "xo/imgui/VulkanApp.hpp"
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <iostream>

namespace {
    ImDrawData *
    imgui_draw_frame(ImGuiContext * imgui_cx)
    {
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
            ++counter;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        return draw_data;
    }
}

int main() {
    VulkanApp app;

    app.assign_imgui_draw_frame(imgui_draw_frame);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* end imgui_ex4.cpp */
