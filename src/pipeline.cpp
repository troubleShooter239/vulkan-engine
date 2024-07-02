#include "include/pipeline.hpp"

#include <fstream>
#include <stdexcept>

#include "pipeline.hpp"

std::vector<char> Pipeline::ReadFile(const char* filePath) {
    // std::string enginePath = ENGINE_DIR + filepath;
    std::ifstream file{filePath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) throw std::runtime_error("failed to open file: " + *filePath);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
void Pipeline::CreateGraphicsPipeline(const char* vertPath, const char* fragPath) {}
