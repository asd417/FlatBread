#include "window.h"
#include <exception>
#include <stdexcept>

Window::Window(int _width, int _height, const char* windowName) : width{ _width }, height{ _height } {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height)
{
	auto bglWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	bglWindow->frameBufferResized = true;
	bglWindow->width = width;
	bglWindow->height = height;
}
