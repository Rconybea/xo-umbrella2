/* VulkanApp.cpp */

#include "VulkanApp.hpp"

void
MinimalImGuiVulkan::run() {
    this->init_sdl_window();
    this->init_vulkan();
    this->init_imgui();
    this->main_loop();
    this->cleanup();
}

void
MinimalImGuiVulkan::init_sdl_window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Failed to initialize SDL!");
    }

    this->window_ = SDL_CreateWindow(
        "ImGui Vulkan SDL2 Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000, 800,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
        );

    if (!window_) {
        throw std::runtime_error("Failed to create SDL window!");
    }
}

void
MinimalImGuiVulkan::init_vulkan() {
    this->create_instance();
    this->create_surface();
    this->pick_physical_device();
    this->create_logical_device();
    this->createSwapchain();
    this->createImageViews();
    this->createRenderPass();   // must come before createFrameBuffers
    this->createFramebuffers();
    this->create_command_pool();
    this->create_command_buffers();
    this->create_sync_objects();
    this->create_descriptor_pool();
}

/* end VulkanApp.cpp */
