#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
   public:
	Window(const int width, const int height, const char *name);
	~Window();

	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

	bool ShouldClose();
	VkExtent2D GetExtent();
	bool IsResized() const;
	void ResetResized();
	GLFWwindow *GetGLFWwindow() const;
	void CreateWindowSurface(const VkInstance &instance, VkSurfaceKHR *surface);

   private:
	static void FrameBufferResizeCallback(GLFWwindow *window, const int width, const int height);
	void InitWindow();

	int _width;
	int _height;
	bool _frameBufferResized = false;

	const char *_windowName;
	GLFWwindow *_window;
};

#endif