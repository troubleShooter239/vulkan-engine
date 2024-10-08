#include "vulkan_ge.hpp"

#include <array>
#include <stdexcept>

VulkanGE::VulkanGE(const int width, const int height, const char* appName)
    : _window{width, height, appName}, _device{_window}, _swapChain{_device, _window.GetExtent()} {
    LoadModels();
    CreatePipeline();
    CreatePipelineLayout();
    CreateCommandBuffers();
}

VulkanGE::~VulkanGE() { vkDestroyPipelineLayout(_device.GetDevice(), _pipelineLayout, nullptr); }

void VulkanGE::Run() {
    while (!_window.ShouldClose()) {
        glfwPollEvents();
        DrawFrame();
    }
    vkDeviceWaitIdle(_device.GetDevice());
}

void VulkanGE::LoadModels() {
    std::vector<Model::Vertex> verticies{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
    _model = std::make_unique<Model>(_device, verticies);
}

void VulkanGE::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(_device.GetDevice(), &layoutInfo, nullptr, &_pipelineLayout) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

void VulkanGE::CreatePipeline() {
    PipelineConfigInfo configInfo{};
    Pipeline::DefaultPipelineConfigInfo(configInfo, _swapChain.GetWidth(), _swapChain.GetHeight());
    configInfo.RenderPass = _swapChain.GetRenderPass();
    configInfo.PipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(
        _device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", configInfo);
}

void VulkanGE::CreateCommandBuffers() {
    _commandBuffers.resize(_swapChain.ImageCount());
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = _device.GetCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    if (vkAllocateCommandBuffers(_device.GetDevice(), &allocateInfo, _commandBuffers.data()) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");

    for (std::size_t i = 0; i < _commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffers");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _swapChain.GetRenderPass();
        renderPassInfo.framebuffer = _swapChain.GetFrameBuffer(i);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _swapChain.GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        _pipeline->Bind(_commandBuffers[i]);
        _model->Bind(_commandBuffers[i]);
        _model->Draw(_commandBuffers[i]);

        vkCmdEndRenderPass(_commandBuffers[i]);
        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
    }
}

void VulkanGE::DrawFrame() {
    uint32_t imageIndex;
    VkResult result = _swapChain.AcquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image");

    result = _swapChain.SubmitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) throw std::runtime_error("failed to present swap chain image");
}
