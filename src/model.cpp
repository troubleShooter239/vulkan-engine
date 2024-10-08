#include "model.hpp"

#include <cassert>
#include <cstring>

Model::Model(Device& device, const std::vector<Vertex>& vertices) : _device{device} {
    CreateVertexBuffers(vertices);
}

Model::~Model() {
    vkDestroyBuffer(_device.GetDevice(), _vertexBuffer, nullptr);
    vkFreeMemory(_device.GetDevice(), _vertexBufferMemory, nullptr);
}

void Model::CreateVertexBuffers(const std::vector<Vertex>& vertices) {
    _vertexCount = static_cast<uint32_t>(vertices.size());
    assert(_vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;
    _device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _vertexBuffer,
        _vertexBufferMemory);

    void* data;
    vkMapMemory(_device.GetDevice(), _vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_device.GetDevice(), _vertexBufferMemory);
}

void Model::Bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void Model::Draw(VkCommandBuffer commandBuffer) { vkCmdDraw(commandBuffer, _vertexCount, 1, 0, 0); }

std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions() {
    return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions() {
    return {{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, Position)},
        {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Color)}};
}
