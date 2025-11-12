/* VulkanApp.cpp */

#include "VulkanApp.hpp"

VulkanApp::VulkanApp(ImguiDrawFn draw_fn)
    : imgui_draw_frame_{draw_fn}
{}

void
VulkanApp::setup(std::function<void (ImGuiContext *)> load_fonts)
{
    if (!setup_flag_) {
        this->setup_flag_ = true;

        this->init_sdl_window();
        this->init_vulkan();
        this->init_imgui(std::move(load_fonts));
    }
}

void
VulkanApp::run(std::function<void (ImGuiContext *)> load_fonts)
{
    this->setup(std::move(load_fonts));

    this->main_loop();

    this->cleanup();
}

void
VulkanApp::init_sdl_window()
{
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
VulkanApp::init_vulkan()
{
    this->create_instance();
    this->create_surface();
    this->pick_physical_device();
    this->create_logical_device();
    this->create_xswapchain(true /*create_render_pass_flag*/);
    this->create_command_pool();
    this->create_command_buffers();
    this->create_sync_objects();
    this->create_descriptor_pool();
}

void
VulkanApp::create_instance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ImGui Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, nullptr)) {
        throw std::runtime_error("Failed to get SDL Vulkan extensions!");
    }

    std::vector<const char*> extensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, extensions.data())) {
        throw std::runtime_error("Failed to get SDL Vulkan extensions!");
    }

#ifdef __apple__
    // Add portability extension for MoltenVK (macOS)
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

#ifdef __apple__
    // CRITICAL: Enable portability enumeration flag for MoltenVK
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    int result = vkCreateInstance(&createInfo, nullptr, &(this->instance_));
    if (result != VK_SUCCESS) {
        printf("vkCreateInstance failed with error: %d\n", result);
        throw std::runtime_error("Failed to create instance!");
    }
}

void
VulkanApp::create_surface()
{
    if (!SDL_Vulkan_CreateSurface(window_, instance_, &(this->surface_))) {
        throw std::runtime_error("Failed to create SDL Vulkan surface!");
    }
}

void
VulkanApp::pick_physical_device()
{
    uint32_t n_device = 0;
    vkEnumeratePhysicalDevices(instance_, &n_device, nullptr);

    if (n_device == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(n_device);
    vkEnumeratePhysicalDevices(instance_, &n_device, devices.data());

    this->physical_device_ = devices[0]; // Just pick the first one for simplicity

    // Find graphics queue family
    uint32_t n_queue_family = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &n_queue_family, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(n_queue_family);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &n_queue_family, queue_families.data());

    for (uint32_t i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &present_support);
            if (present_support) {
                this->graphics_queue_family_ = i;
                break;
            }
        }
    }
}

void
VulkanApp::create_logical_device()
{
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_queue_family_;
    queue_create_info.queueCount = 1;
    float queuePriority = 1.0f;
    queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.queueCreateInfoCount = 1;
    create_info.pEnabledFeatures = &deviceFeatures;

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(physical_device_, &create_info, nullptr, &(this->device_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
}

void
VulkanApp::create_swapchain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    uint32_t n_format;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &n_format, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(n_format);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &n_format, formats.data());

    VkSurfaceFormatKHR surface_format = formats[0];
    this->swapchain_image_format_ = surface_format.format;

    int width, height;
    SDL_Vulkan_GetDrawableSize(window_, &width, &height);
    this->swapchain_extent_ = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    uint32_t n_image = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && n_image > capabilities.maxImageCount) {
        n_image = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;
    create_info.minImageCount = n_image;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swapchain_extent_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = (vsync_enabled_flag_ ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR);
    create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &(this->swapchain_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_, swapchain_, &n_image, nullptr);
    this->swapchain_images_.resize(n_image);
    vkGetSwapchainImagesKHR(device_, swapchain_, &n_image, this->swapchain_images_.data());
}

void
VulkanApp::create_image_views()
{
    swapchain_image_views_.resize(swapchain_images_.size());

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_images_[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_image_format_;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &create_info, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void
VulkanApp::create_render_pass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain_image_format_;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &(this->render_pass_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void
VulkanApp::create_framebuffers()
{
    framebuffers_.resize(swapchain_image_views_.size());

    for (size_t i = 0; i < swapchain_image_views_.size(); i++) {
        VkImageView attachments[] = { swapchain_image_views_[i] };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = swapchain_extent_.width;
        framebuffer_info.height = swapchain_extent_.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &(this->framebuffers_[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void
VulkanApp::create_command_pool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphics_queue_family_;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &(this->command_pool_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void
VulkanApp::create_command_buffers()
{
    command_buffers_.resize(c_max_frames_in_flight);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void
VulkanApp::create_sync_objects()
{
    this->image_available_semaphores_.resize(c_max_frames_in_flight);
    this->render_finished_semaphores_.resize(c_max_frames_in_flight);
    this->inflight_fences_.resize(c_max_frames_in_flight);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < c_max_frames_in_flight; i++) {
        if (vkCreateSemaphore(device_, &semaphore_info, nullptr, &(this->image_available_semaphores_[i])) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphore_info, nullptr, &(this->render_finished_semaphores_[i])) != VK_SUCCESS ||
            vkCreateFence(device_, &fence_info, nullptr, &(this->inflight_fences_[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects!");
        }
    }
}

void
VulkanApp::create_descriptor_pool()
{
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &(this->descriptor_pool_)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void
VulkanApp::init_imgui(std::function<void (ImGuiContext *)> load_fonts)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    this->imgui_cx_ = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (load_fonts)
        load_fonts(imgui_cx_);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(window_);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance        = instance_;
    init_info.PhysicalDevice  = physical_device_;
    init_info.Device          = device_;
    init_info.QueueFamily     = graphics_queue_family_;
    init_info.Queue           = graphics_queue_;
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = descriptor_pool_;
    init_info.RenderPass      = render_pass_;
    init_info.Subpass         = 0;
    init_info.MinImageCount   = c_max_frames_in_flight;
    init_info.ImageCount      = static_cast<uint32_t>(swapchain_images_.size());
    init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator       = nullptr;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info);
    //ImGui_ImplVulkan_Init(&init_info, render_pass_);

    // Upload Fonts
    VkCommandBuffer command_buffer = begin_single_time_commands();
    ImGui_ImplVulkan_CreateFontsTexture();
    this->end_single_time_commands(command_buffer);
    ImGui_ImplVulkan_DestroyFontsTexture();
}

VkCommandBuffer
VulkanApp::begin_single_time_commands()
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmdbuf;
    vkAllocateCommandBuffers(device_, &alloc_info, &cmdbuf);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdbuf, &begin_info);

    return cmdbuf;
}

void
VulkanApp::end_single_time_commands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);

    vkFreeCommandBuffers(device_, command_pool_, 1, &commandBuffer);
}

void
VulkanApp::main_loop()
{
    SDL_Event event;

    while (!quit_flag_) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) {
                this->quit_flag_ = true;
            }
        }

        this->draw_frame();
    }

    vkDeviceWaitIdle(device_);
}

void
VulkanApp::draw_frame()
{
    vkWaitForFences(device_, 1, &inflight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    uint32_t image_ix = 0;
    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                            image_available_semaphores_[current_frame_],
                                            VK_NULL_HANDLE, &image_ix);

    switch (result) {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        this->recreate_xswapchain();
        // deliberate earlyexit
        return;
    default:
        throw std::runtime_error("failed to acquire swapchain image!");
        break;
    }

    vkResetFences(device_, 1, &inflight_fences_[current_frame_]);

    vkResetCommandBuffer(command_buffers_[current_frame_], 0);
    this->record_command_buffer(command_buffers_[current_frame_], image_ix);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers_[current_frame_];

    VkSemaphore signalSemaphores[] = {render_finished_semaphores_[current_frame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, inflight_fences_[current_frame_]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &image_ix;

    result = vkQueuePresentKHR(graphics_queue_, &presentInfo);

    if (xswapchain_recreate_flag_)
        result = VK_ERROR_OUT_OF_DATE_KHR;

    switch (result) {
    case VK_SUCCESS:
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        this->recreate_xswapchain();
        break;
    default:
        throw std::runtime_error("failed to present swapchain image!");
    }

    this->current_frame_ = (current_frame_ + 1) % c_max_frames_in_flight;
}

void
VulkanApp::update_vsync_enabled(bool flag) {
    this->vsync_enabled_flag_ = flag;
    this->xswapchain_recreate_flag_ = true;
}

void
VulkanApp::record_command_buffer(VkCommandBuffer cmdbuf, uint32_t image_ix)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmdbuf, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = framebuffers_[image_ix];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain_extent_;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(cmdbuf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

#ifdef OBSOLETE
    // Start the Dear ImGui frame
    {
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
        ImGui_ImplVulkan_RenderDrawData(draw_data, cmdbuf);

    }
#endif

    ImDrawData * draw_data = imgui_draw_frame_(this, imgui_cx_);

    ImGui_ImplVulkan_RenderDrawData(draw_data, cmdbuf);

    vkCmdEndRenderPass(cmdbuf);

    if (vkEndCommandBuffer(cmdbuf) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void
VulkanApp::create_xswapchain(bool create_render_pass_flag)
{
    this->create_swapchain();
    this->create_image_views();
    if(create_render_pass_flag)
        this->create_render_pass();
    this->create_framebuffers();
}

void
VulkanApp::cleanup_xswapchain()
{
    this->cleanup_framebuffers();
    this->cleanup_image_views();
    this->cleanup_swapchain();
}

void
VulkanApp::recreate_xswapchain()
{
    // handle window minimization: wait until window has valid size
    this->wait_not_minimized();

    // wait until device idle before cleaning up resources
    vkDeviceWaitIdle(device_);

    // remember consistent swapchain state restored
    this->xswapchain_recreate_flag_ = false;

    // cleanup old xswapchain
    this->cleanup_xswapchain();

    // create new swapchain
    this->create_xswapchain(false /*create_render_pass_flag*/);
}

void
VulkanApp::wait_not_minimized()
{
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window_, &width, &height);
    while (width == 0 || height == 0) {
        SDL_GetWindowSize(window_, &width, &height);
        SDL_WaitEvent(nullptr);
    }
}

void
VulkanApp::cleanup_framebuffers()
{
    for (auto framebuffer : framebuffers_) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }
    framebuffers_.clear();
}

void
VulkanApp::cleanup_image_views()
{
    for (auto imageView : swapchain_image_views_) {
        vkDestroyImageView(device_, imageView, nullptr);
    }
    swapchain_image_views_.clear();
}

void
VulkanApp::cleanup_swapchain()
{
    vkDestroySwapchainKHR(device_, this->swapchain_, nullptr);
}

void
VulkanApp::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    for (size_t i = 0; i < c_max_frames_in_flight; i++) {
        vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
        vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        vkDestroyFence(device_, inflight_fences_[i], nullptr);
    }

    vkDestroyCommandPool(device_, command_pool_, nullptr);

    this->cleanup_xswapchain();

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

/* end VulkanApp.cpp */
