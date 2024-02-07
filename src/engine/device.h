#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "window.h"

#include <set>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_set>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device {
public:
    Device(Window& window) : _window{ window } {
        createInstance();
        setupDebugMessenger();
        _window.createWindowSurface(instance, &surface);
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }
    ~Device() {}

    static VkDevice& device() { return _device; }
    VkCommandPool getCommandPool() const { return commandPool; }
    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
    VkSurfaceKHR getSurface() const { return surface; }

    VkBool32 Device::getSupportedDepthsFormat(VkFormat* depthFormat);

    VkQueue graphicsQueue() const { return graphicsQueue_; }
    VkQueue presentQueue() const { return presentQueue_; }

    void createImageWithInfo(
        const VkImageCreateInfo& imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

private:
    static VkDevice _device;

    struct ImmediateUploadContext {
        VkFence _uploadFence;
        VkCommandPool _commandPool;
        VkCommandBuffer _commandBuffer;
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void createInstance();
    std::vector<const char*> getRequiredExtensions();
    void hasGflwRequiredInstanceExtensions();

    //Debugger
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    bool checkValidationLayerSupport();
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    void createLogicalDevice();
    void createCommandPool();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();

    const bool enableValidationLayers = true;
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    Window& _window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceFeatures supportedFeatures;
    VkPhysicalDeviceProperties properties;

    VkCommandPool commandPool;
};