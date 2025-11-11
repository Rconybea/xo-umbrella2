/* VulkanApp.cpp */

#include "VulkanApp.hpp"

void
MinimalImGuiVulkan::run()
{
    this->init_sdl_window();
    this->init_vulkan();
    this->init_imgui();
    this->main_loop();
    this->cleanup();
}

void
MinimalImGuiVulkan::init_sdl_window()
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
MinimalImGuiVulkan::init_vulkan()
{
    this->create_instance();
    this->create_surface();
    this->pick_physical_device();
    this->create_logical_device();
    this->create_swapchain();
    this->create_image_views();
    this->create_render_pass();   // must come before createFrameBuffers
    this->create_framebuffers();
    this->create_command_pool();
    this->create_command_buffers();
    this->create_sync_objects();
    this->create_descriptor_pool();
}

void
MinimalImGuiVulkan::create_instance()
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
MinimalImGuiVulkan::create_surface()
{
    if (!SDL_Vulkan_CreateSurface(window_, instance_, &(this->surface_))) {
        throw std::runtime_error("Failed to create SDL Vulkan surface!");
    }
}

void
MinimalImGuiVulkan::pick_physical_device()
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
MinimalImGuiVulkan::create_logical_device()
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

/* end VulkanApp.cpp */
