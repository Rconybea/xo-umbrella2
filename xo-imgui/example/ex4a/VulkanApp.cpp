/* VulkanApp.cpp */

#include "VulkanApp.hpp"

void MinimalImGuiVulkan::run()
{
    this->initWindow();
    this->initVulkan();
    this->initImGui();
    this->mainLoop();
    this->cleanup();
}

/* end VulkanApp.cpp */
