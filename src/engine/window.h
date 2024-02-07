#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
	Window(int width, int height, const char* windowName);
    ~Window() {};
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    GLFWwindow* window;

    Window(const Window&) = delete;
    Window& operator= (const Window&) = delete;
    bool wasWindowResized() const { return frameBufferResized; }

    VkExtent2D getExtent() { return { static_cast<uint32_t> (width), static_cast<uint32_t> (height) }; }
    void resetWindowResizedFlag() { frameBufferResized = false; };
    bool shouldClose() { return glfwWindowShouldClose(window); }
private:
    static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
    bool frameBufferResized = false;
    int width;
    int height;
};