#include "include/window.hpp"

#include <stdexcept>

#include "window.hpp"

Window::Window(const int width, const int height, const char* name) { InitWindow(); }

Window::~Window() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

bool Window::ShouldClose() { return glfwWindowShouldClose(_window); }

VkExtent2D Window::GetExtent() {
    { return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}; }
}

bool Window::IsResized() const { return _frameBufferResized; }

void Window::ResetResized() { _frameBufferResized = false; }

GLFWwindow* Window::GetGLFWwindow() const { return _window; }

void Window::CreateWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface");
}

void Window::FrameBufferResizeCallback(GLFWwindow* window, const int width, const int height) {
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->_frameBufferResized = true;
    w->_width = width;
    w->_height = height;
}

void Window::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow(_width, _height, _windowName, nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, FrameBufferResizeCallback);
}
