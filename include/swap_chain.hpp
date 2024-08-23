#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "device.hpp"

class SwapChain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain &operator=(const SwapChain &) = delete;

    VkFramebuffer GetFrameBuffer(int index);
    VkRenderPass GetRenderPass();
    VkImageView GetImageView(int index);
    size_t ImageCount() noexcept;
    VkFormat GetSwapChainImageFormat();
    VkExtent2D GetSwapChainExtent();
    uint32_t GetWidth() const noexcept;
    uint32_t GetHeight() const noexcept;

    float ExtentAspectRatio() noexcept;
    VkFormat FindDepthFormat();

    VkResult AcquireNextImage(uint32_t *imageIndex);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

private:
    void CreateSwapChain();
    void CreateImageViews();
    void CreateDepthResources();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat _swapChainImageFormat;
    VkExtent2D _swapChainExtent;

    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkRenderPass _renderPass;

    std::vector<VkImage> _depthImages;
    std::vector<VkDeviceMemory> _depthImageMemories;
    std::vector<VkImageView> _depthImageViews;
    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;

    Device &_device;
    VkExtent2D _windowExtent;

    VkSwapchainKHR _swapChain;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    size_t _currentFrame = 0;
};
