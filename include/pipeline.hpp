#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vector>

class Pipeline {
   public:
   private:
	static std::vector<char> ReadFile(const char* filePath);

	void CreateGraphicsPipeline(const char* vertPath, const char* fragPath);
};

#endif