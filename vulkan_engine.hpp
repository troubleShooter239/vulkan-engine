#ifndef VULKAN_ENGINE_HPP
#define VULKAN_ENGINE_HPP

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

class VulkanEngine {
 public:
  VulkanEngine(const unsigned short width, const unsigned short height,
               const char* title = "VulkanApp", GLFWmonitor* monitor = nullptr);
  ~VulkanEngine() noexcept;
  void Run() const;

 private:
  GLFWwindow* _window;
  VkPhysicalDevice _physicalDevice;
  VkDevice _device;
  VkInstance _instance;

  void CreateInstance();
  void CreateLogicalDevice();
  void PickPhysicalDevice();
  std::optional<uint32_t> FindQueueFamilies(VkPhysicalDevice& device);
};

#endif