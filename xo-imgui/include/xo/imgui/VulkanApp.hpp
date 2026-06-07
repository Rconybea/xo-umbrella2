#pragma once

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>
#include <iostream>
#include <functional>
#include <vector>
#include <stdexcept>

class VulkanApp {
public:
    using ImguiDrawFn = std::function<ImDrawData * (VulkanApp *, ImGuiContext *)>;

public:
    /* create single-window vulkan application;
     * use @p fn to draw each frame
     */
    VulkanApp(ImguiDrawFn fn);

    bool vsync_enabled_flag() const { return vsync_enabled_flag_; }

    /* update vsync enabled (will recreate xswapchain at next opportunity)
     */
    void update_vsync_enabled(bool flag);

    /* run application; borrows calling thread for event loop,
     * until application exit.
     * Invoke load_fonts once imgui context established
     * @ref vsync_enabled_flag_. true for VK_PRESENT_MODE_FIFO_KHR;
     * false for VK_PRESENT_MODE_IMMEDIATE_KHR
     */
    void run(std::function<void (ImGuiContext *)> load_fonts);

    /* application setup (sdl -> vulkan -> imgui)
     * Invoke load_fonts once imgui context established.
     */
    void setup(std::function<void (ImGuiContext *)> load_fonts);

    /* borrow calling thread, run until application end
     * Require: app state initialized. see init_sdl_window(), init_vulkan(), init_imgui()
     */
    void main_loop();

    /* orderly shutdown
     */
    void cleanup();

private:
    /* create SDL window for application.
     * populates @ref window_
     */
    void init_sdl_window();

    /* setup vulkan state. swapchain, command buffers etc
     * @ref vsync_enabled_flag_. true for VK_PRESENT_MODE_FIFO_KHR;
     * false for VK_PRESENT_MODE_IMMEDIATE_KHR
     */
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

    /* populates @ref swapchain_, @ref swapchain_images_,
     * @ref swapchain_image_views_, @ref framebuffers_.
     * Also $ref render_pass_ iff @p create_render_pass_flag
     * @ref vsync_enabled_flag_. true for VK_PRESENT_MODE_FIFO_KHR;
     * false for VK_PRESENT_MODE_IMMEDIATE_KHR
     */
    void create_xswapchain(bool create_render_pass_flag);

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

    /* Setup imgui "framework".
     * Invoke @p load_fonts with ImGuiContext
     */
    void init_imgui(std::function<void (ImGuiContext *)> load_fonts);

    /* Allocate buffer for 'single-time' commands (to be run once?).
     * Pair with call to end_single_time_commands()
     */
    VkCommandBuffer begin_single_time_commands();

    /* complete command buffer begun with begin_single_time_commands();
     * also submit, wait for completion + cleanup
     */
    void end_single_time_commands(VkCommandBuffer commandBuffer);

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

    /* Teardown + create swapchain (swapchain + framebuffers + image views).
     * Need this after window size changes
     */
    void recreate_xswapchain();

    /* wait until non-minimized window */
    void wait_not_minimized();

    /* orderly disposal of swapchin + image_views + framebuffers */
    void cleanup_xswapchain();

    /* orderly disposal of @ref framebuffers_ */
    void cleanup_framebuffers();

    /* orderly disposal of @ref swapchain_image_views_ */
    void cleanup_image_views();

    /* orderly disposal of @ref swapchain_ */
    void cleanup_swapchain();

private:
    SDL_Window* window_ = nullptr;
    ImGuiContext* imgui_cx_ = nullptr;

    VkInstance instance_;

    /* abstraction for presentation area (?) */
    VkSurfaceKHR surface_;
    /* true -> VK_PRESENT_MODE_FIFO_KHR
     * false -> VK_PRESENT_MODE_IMMEDIATE_KHR
     */
    bool vsync_enabled_flag_ = true;

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
    const size_t c_max_frames_in_flight = 2;
    /* true iff @ref setup has been called */
    bool setup_flag_ = false;
    /* true if xswapchain is out-of-date.
     * reset when xswapchain recreated (and therefore in consistent state).
     *
     * Triggered by:
     * - window resize
     * - vsync toggled
     */
    bool xswapchain_recreate_flag_ = false;
    bool quit_flag_ = false;

    /** draw imgui **/
    const ImguiDrawFn imgui_draw_frame_;
}; /*VulkanApp*/
