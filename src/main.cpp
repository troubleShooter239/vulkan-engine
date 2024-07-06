#include <iostream>

#include "include/vulkan_ge.hpp"

int main() {
    VulkanGE gameEngine{800, 600, "App"};
    try {
        gameEngine.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
