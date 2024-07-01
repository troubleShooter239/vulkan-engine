#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "include/vulkan_engine.hpp"

int main() {
	VulkanEngine engine(800, 600);
	try {
		engine.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}