#pragma once

#include <memory>

#include "device.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

class VulkanGE {
public:
    VulkanGE(const int width, const int height, const char *appName = "App");
    ~VulkanGE();

    VulkanGE(const VulkanGE &) = delete;
    VulkanGE &operator=(const VulkanGE &) = delete;

    void Run();

private:
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void DrawFrame();

    Window _window;
    Device _device;
    SwapChain _swapChain;
    std::unique_ptr<Pipeline> _pipeline;
    VkPipelineLayout _pipelineLayout;
    std::vector<VkCommandBuffer> _commandBuffers;
};
