/* VulkanApp.cpp */

#include "VulkanApp.hpp"

void MinimalImGuiVulkan::run()
{
    this->init_window();
    this->init_vulkan();
    this->init_imgui();
    this->main_loop();
    this->cleanup();
}

/* end VulkanApp.cpp */
