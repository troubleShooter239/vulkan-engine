#pragma once

#include <vector>

class Pipeline {
   public:
	Pipeline();
   private:
	static std::vector<char> ReadFile(const char* filePath);

	void CreateGraphicsPipeline(const char* vertPath, const char* fragPath);
};
