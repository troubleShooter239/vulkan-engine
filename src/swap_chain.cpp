#include "include/swap_chain.hpp"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

SwapChain::SwapChain(Device &deviceRef, VkExtent2D windowExtent)
    : _device{deviceRef}, _windowExtent{windowExtent} {
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDepthResources();
    CreateFramebuffers();
    CreateSyncObjects();
}

SwapChain::~SwapChain() {
    for (VkImageView imageView : _swapChainImageViews)
        vkDestroyImageView(_device.GetDevice(), imageView, nullptr);
    _swapChainImageViews.clear();

    if (_swapChain != nullptr) {
        vkDestroySwapchainKHR(_device.GetDevice(), _swapChain, nullptr);
        _swapChain = nullptr;
    }

    for (int i = 0; i < _depthImages.size(); i++) {
        vkDestroyImageView(_device.GetDevice(), _depthImageViews[i], nullptr);
        vkDestroyImage(_device.GetDevice(), _depthImages[i], nullptr);
        vkFreeMemory(_device.GetDevice(), _depthImageMemories[i], nullptr);
    }

    for (VkFramebuffer framebuffer : _swapChainFramebuffers)
        vkDestroyFramebuffer(_device.GetDevice(), framebuffer, nullptr);

    vkDestroyRenderPass(_device.GetDevice(), _renderPass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_device.GetDevice(), _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device.GetDevice(), _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device.GetDevice(), _inFlightFences[i], nullptr);
    }
}

VkFramebuffer SwapChain::GetFrameBuffer(int index) { return _swapChainFramebuffers[index]; }

VkRenderPass SwapChain::GetRenderPass() { return _renderPass; }

VkImageView SwapChain::GetImageView(int index) { return _swapChainImageViews[index]; }

size_t SwapChain::ImageCount() noexcept { return _swapChainImages.size(); }

VkFormat SwapChain::GetSwapChainImageFormat() { return _swapChainImageFormat; }

VkExtent2D SwapChain::GetSwapChainExtent() { return _swapChainExtent; }

uint32_t SwapChain::GetWidth() const noexcept { return _swapChainExtent.width; }

uint32_t SwapChain::GetHeight() const noexcept { return _swapChainExtent.height; }

float SwapChain::ExtentAspectRatio() noexcept {
    return static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height);
}

VkResult SwapChain::AcquireNextImage(uint32_t *imageIndex) {
    vkWaitForFences(_device.GetDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    VkResult result =
        vkAcquireNextImageKHR(_device.GetDevice(), _swapChain, std::numeric_limits<uint64_t>::max(),
            _imageAvailableSemaphores[_currentFrame],  // must be a not signaled semaphore
            VK_NULL_HANDLE, imageIndex);

    return result;
}

VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if (_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(_device.GetDevice(), 1, &_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);

    _imagesInFlight[*imageIndex] = _inFlightFences[_currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_device.GetDevice(), 1, &_inFlightFences[_currentFrame]);
    if (vkQueueSubmit(_device.GetGraphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    VkResult result = vkQueuePresentKHR(_device.GetPresentQueue(), &presentInfo);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::CreateSwapChain() {
    SwapChainSupportDetails swapChainSupport = _device.GetSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _device.GetSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = _device.FindPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device.GetDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(_device.GetDevice(), _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device.GetDevice(), _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void SwapChain::CreateImageViews() {
    _swapChainImageViews.resize(_swapChainImages.size());
    for (size_t i = 0; i < _swapChainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = _swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device.GetDevice(), &viewInfo, nullptr, &_swapChainImageViews[i]) !=
            VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");
    }
}

void SwapChain::CreateRenderPass() {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = GetSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubPass = {};
    SubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubPass.colorAttachmentCount = 1;
    SubPass.pColorAttachments = &colorAttachmentRef;
    SubPass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};

    dependency.dstSubpass = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &SubPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device.GetDevice(), &renderPassInfo, nullptr, &_renderPass) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void SwapChain::CreateFramebuffers() {
    const size_t imageCount = ImageCount();
    _swapChainFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        std::array<VkImageView, 2> attachments = {_swapChainImageViews[i], _depthImageViews[i]};

        VkExtent2D swapChainExtent = GetSwapChainExtent();
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device.GetDevice(), &framebufferInfo, nullptr,
                &_swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

void SwapChain::CreateDepthResources() {
    VkFormat depthFormat = FindDepthFormat();
    VkExtent2D swapChainExtent = GetSwapChainExtent();

    const size_t imageCount = ImageCount();
    _depthImages.resize(imageCount);
    _depthImageMemories.resize(imageCount);
    _depthImageViews.resize(imageCount);

    for (int i = 0; i < _depthImages.size(); i++) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        _device.CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImages[i],
            _depthImageMemories[i]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device.GetDevice(), &viewInfo, nullptr, &_depthImageViews[i]) !=
            VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");
    }
}

void SwapChain::CreateSyncObjects() {
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(ImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDevice device = _device.GetDevice();
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const VkSurfaceFormatKHR &availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "Present mode: Mailbox\n";
            return availablePresentMode;
        }
    }

    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }

    std::cout << "Present mode: V-Sync\n";
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = _windowExtent;
        actualExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat SwapChain::FindDepthFormat() {
    return _device.FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
