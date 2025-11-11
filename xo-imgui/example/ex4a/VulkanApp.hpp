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

    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render_pass_;
        renderPassInfo.framebuffer = framebuffers_[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain_extent_;

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

    void recreate_swapchain() {
        // handle window minimization: wait until window has valid size
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window_, &width, &height);
        while (width == 0 || height == 0) {
            SDL_GetWindowSize(window_, &width, &height);
            SDL_WaitEvent(nullptr);
        }

        // wait until device idle before cleaning up resources
        vkDeviceWaitIdle(device_);

        // cleanup old swapchain
        this->cleanupFrameBuffers();
        this->cleanupImageViews();
        this->cleanupSwapchain();

        // create new swapchain
        this->create_swapchain();
        this->create_image_views();
        this->create_framebuffers();
    }

    void cleanupFrameBuffers() {
            for (auto framebuffer : framebuffers_) {
                vkDestroyFramebuffer(device_, framebuffer, nullptr);
            }
            framebuffers_.clear();
    }

    void cleanupImageViews() {
            for (auto imageView : swapchain_image_views_) {
                vkDestroyImageView(device_, imageView, nullptr);
            }
            swapchain_image_views_.clear();
    }

    void cleanupSwapchain() {
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

        this->cleanupFrameBuffers();
        this->cleanupImageViews();
        this->cleanupSwapchain();

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
