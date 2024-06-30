#include "vulkan_engine.hpp"

#include <map>
#include <optional>
#include <vector>

VulkanEngine::VulkanEngine(const unsigned short width,
                           const unsigned short height, const char* title,
                           GLFWmonitor* monitor) {
  // glfw init
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  _window = glfwCreateWindow(width, height, title, monitor, nullptr);

  // vulkan init
  CreateInstance();
  _physicalDevice = VK_NULL_HANDLE;
  PickPhysicalDevice();
  CreateLogicalDevice();
}

VulkanEngine::~VulkanEngine() noexcept {
  vkDestroyInstance(_instance, nullptr);
  glfwDestroyWindow(_window);
  glfwTerminate();
}

void VulkanEngine::Run() const {
  while (!glfwWindowShouldClose(_window)) glfwPollEvents();
}

void VulkanEngine::CreateInstance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t glfwExtensionCount = 0;

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  createInfo.enabledLayerCount = 0;

  if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
    throw std::runtime_error("failed to create instance!");
}

void VulkanEngine::CreateLogicalDevice() {
  std::optional<u_int32_t> indices = FindQueueFamilies(_physicalDevice);

  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = indices.value();
  queueCreateInfo.queueCount = 1;
}

void VulkanEngine::PickPhysicalDevice() {
  uint32_t deviceCount = 0;

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

  std::multimap<const int&, const VkPhysicalDevice&> candidates;

  for (const VkPhysicalDevice& device : devices) {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    // Application can't function without geometry shaders
    if (!features.geometryShader) {
      candidates.insert(std::make_pair(0, device));
      continue;
    }

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      score += 1000;

    // Maximum possible size of textures affects graphics quality
    score += properties.limits.maxImageDimension2D;

    candidates.insert(std::make_pair(score, device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0)
    _physicalDevice = candidates.rbegin()->second;
  else
    throw std::runtime_error("failed to find a suitable GPU!");
}

std::optional<uint32_t> VulkanEngine::FindQueueFamilies(
    VkPhysicalDevice& device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  std::optional<uint32_t> graphicsFamily;
  int i = 0;
  for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsFamily = i;

    if (graphicsFamily.has_value()) break;

    i++;
  }

  return graphicsFamily;
}