/* imgui_ex4.cpp */

#include "xo/imgui/VulkanApp.hpp"
#include <iostream>

int main() {
    VulkanApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* end imgui_ex4.cpp */
