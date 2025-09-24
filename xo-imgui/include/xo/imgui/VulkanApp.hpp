/* file VulkanApp.hpp */

#pragma once

#include <SDL.h>
#include <imgui.h>
#include <vulkan/vulkan.h>
//#include <SDL_vulkan.h>
#include <vector>
#include <functional>

class VulkanApp {
public:
    using ImguiDrawFn = std::function<ImDrawData * (ImGuiContext *)>;

public:
    VulkanApp(ImguiDrawFn fn);

#ifdef NOPE
    /** set imgui draw function **/
    void assign_imgui_draw_frame(ImguiDrawFn fn);
#endif

    /** equivalent to sequence setup(), main_loop(), cleanup() **/
    void run();

    /** setup before main loop.  idempotent **/
    void setup(std::function<void (ImGuiContext *)> load_fonts);
    void main_loop();
    /** cleanup before shutdown.  idempotent **/
    void cleanup();

private:
    void init_window();
    void init_vulkan();
    void create_instance();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_framebuffers();
    void create_command_pool();
    void create_command_buffers();
    void create_sync_objects();
    void create_descriptor_pool();

    void cleanup_framebuffers();
    void cleanup_render_pass();
    void cleanup_image_views();
    void cleanup_swapchain();

    void cleanup_swapchain_deps();
    void recreate_swapchain_deps();

    void init_imgui(std::function<void (ImGuiContext *)> load_fonts);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer commandBuffer);
    void record_command_buffer(VkCommandBuffer commandBuffer,
                               uint32_t imageIndex);
    /** TODO: replace with some generic mechanism **/
    void draw_frame();

private:
    bool setup_done_ = false;
    bool cleanup_done_ = false;

    SDL_Window* window_ = nullptr;
    ImGuiContext* imgui_cx_ = nullptr;
    VkInstance instance;
    VkPhysicalDevice physical_device_;
    VkDevice device_;
    VkQueue graphics_queue_;
    VkSurfaceKHR surface_;
    VkSwapchainKHR swapchain_;
    VkFormat swapchain_image_format_;
    VkExtent2D swapchain_extent_;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchain_image_views_;
    VkRenderPass render_pass_;
    std::vector<VkFramebuffer> framebuffers_;
    VkCommandPool command_pool_;
    std::vector<VkCommandBuffer> command_buffers_;
    /** one per frame (up to MAX_FRAMES_IN_FLIGHT) **/
    std::vector<VkSemaphore> image_available_semaphores_;
    /** one per frame (up to MAX_FRAMES_IN_FLIGHT) **/
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    VkDescriptorPool descriptor_pool_;

    /** image index of current frame **/
    uint32_t current_frame_ = 0;
    uint32_t graphics_queue_family_ = 0;
    bool quit_ = false;

    /** draw imgui **/
    const ImguiDrawFn imgui_draw_frame_;
};

/* end VulkanApp.hpp */
