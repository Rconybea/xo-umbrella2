#pragma once

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
    /* run application; borrows calling thread for event loop,
     * until application exit
     */
    void run();

private:
    /* create SDL window for application.
     * populates @ref window_
     */
    void init_sdl_window();

    /* setup vulkan state. swapchain, command buffers etc */
    void init_vulkan();

    /* create vulkan instance.
     * populates @ref instance_
     */
    void create_instance();

    /* create vulkan surface.
     * populates @ref surface_
     */
    void create_surface();

    /* choose physical device (1:1 with graphics card, presumably)
     * populates @ref physical_device_, @ref graphics_queue_family_
     */
    void pick_physical_device();

    /*
     * require: pick_physical_device() has run successfully
     * populates @ref device_, @ref graphics_queue_
     */
    void create_logical_device();

    /*
     * populates @ref swapchain_, @ref swapchain_images_
     */
    void create_swapchain();

    /*
     * populate @ref swapchain_image_views_
     */
    void create_image_views();

    /*
     * populate @ref render_pass_
     */
    void create_render_pass();

    /*
     * populate @ref framebuffers_
     */
    void create_framebuffers();

    /*
     * populate @ref command_pool_
     */
    void create_command_pool();

    /*
     * populate @ref command_buffers_
     */
    void create_command_buffers();

    /*
     * populate
     *   @ref image_available_semaphores_,
     *   @ref render_finished_semaphores_,
     *   @ref inflight_fences_
     */
    void create_sync_objects();

    /*
     * populate @ref descriptor_pool_
     */
    void create_descriptor_pool();

    /* setup imgui "framework" */
    void init_imgui();

    /* Allocate buffer for 'single-time' commands (to be run once?).
     * Pair with call to end_single_time_commands()
     */
    VkCommandBuffer begin_single_time_commands();

    /* complete command buffer begun with begin_single_time_commands();
     * also submit, wait for completion + cleanup
     */
    void end_single_time_commands(VkCommandBuffer commandBuffer);

    /* borrow calling thread, run until application end
     * Require: app state initialized. see init_sdl_window(), init_vulkan(), init_imgui()
     */
    void main_loop();

    /* Accumulate drawing command for next frame.
     * Reserves and modifies the @ref current_frame_ element of
     *   @ref command_buffers_
     *   @ref image_available_semaphores_
     *   @ref inflight_fences_
     * Advances current_frame_ so that it refers to available resources
     */
    void draw_frame();

    /* record draw instructions into cmdbuf, framebuffers_[image_ix] */
    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t image_ix);

    void recreate_swapchain() {
        // handle window minimization: wait until window has valid size
        this->wait_not_minimized();

        // wait until device idle before cleaning up resources
        vkDeviceWaitIdle(device_);

        // cleanup old swapchain
        this->cleanup_framebuffers();
        this->cleanup_image_views();
        this->cleanup_swapchain();

        // create new swapchain
        this->create_swapchain();
        this->create_image_views();
        this->create_framebuffers();
    }

    void wait_not_minimized() {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window_, &width, &height);
        while (width == 0 || height == 0) {
            SDL_GetWindowSize(window_, &width, &height);
            SDL_WaitEvent(nullptr);
        }
    }

    void cleanup_framebuffers() {
            for (auto framebuffer : framebuffers_) {
                vkDestroyFramebuffer(device_, framebuffer, nullptr);
            }
            framebuffers_.clear();
    }

    void cleanup_image_views() {
            for (auto imageView : swapchain_image_views_) {
                vkDestroyImageView(device_, imageView, nullptr);
            }
            swapchain_image_views_.clear();
    }

    void cleanup_swapchain() {
        vkDestroySwapchainKHR(device_, this->swapchain_, nullptr);
    }

    void cleanup() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
            vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
            vkDestroyFence(device_, inflight_fences_[i], nullptr);
        }

        vkDestroyCommandPool(device_, command_pool_, nullptr);

        this->cleanup_framebuffers();
        this->cleanup_image_views();
        this->cleanup_swapchain();

        vkDestroyRenderPass(device_, render_pass_, nullptr);
        vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, this->surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
        this->instance_ = nullptr;

        SDL_DestroyWindow(window_);
        this->window_ = nullptr;

        SDL_Quit();
    }

private:
    SDL_Window* window_ = nullptr;

    VkInstance instance_;

    /* abstraction for presentation area (?) */
    VkSurfaceKHR surface_;

    /* physical device (graphics card) */
    VkPhysicalDevice physical_device_;
    uint32_t graphics_queue_family_ = 0;

    /* logical device (graphics card, abstract api (?)) */
    VkDevice device_;
    VkQueue graphics_queue_;

    /* drawing state, dependent on window size */
    VkFormat swapchain_image_format_;
    VkExtent2D swapchain_extent_;
    VkSwapchainKHR swapchain_;
    std::vector<VkImage> swapchain_images_;

    std::vector<VkImageView> swapchain_image_views_;

    VkRenderPass render_pass_;

    std::vector<VkFramebuffer> framebuffers_;

    VkCommandPool command_pool_;

    std::vector<VkCommandBuffer> command_buffers_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> inflight_fences_;

    VkDescriptorPool descriptor_pool_;

    uint32_t current_frame_ = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    /* true when window resize behavior detected,
     * until swapchain in consistent state.
     */
    bool framebuffer_resized_flag_ = false;
    bool quit_flag_ = false;
}; /*MinimalImGuiVulkan*/
