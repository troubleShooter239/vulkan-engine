#ifndef VULKAN_ENGINE_HPP
#define VULKAN_ENGINE_HPP

#include <optional>
#include <vector>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete();
};

class VulkanEngine {
   public:
	VulkanEngine(const unsigned short width, const unsigned short height,
				 const char* title = "VulkanApp", GLFWmonitor* monitor = nullptr);
	~VulkanEngine() noexcept;

	void Run() const;

   private:
	GLFWwindow* _window;

	VkInstance _instance;
	VkSurfaceKHR _surface;

	VkDevice _device;
	VkPhysicalDevice _physicalDevice;

	VkQueue _presentQueue;
	VkQueue _graphicsQueue;

	void CreateInstance();
	void CreateLogicalDevice();
	void CreateSurface();

	void PickPhysicalDevice();

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool IsDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice& device);
};

#endif
