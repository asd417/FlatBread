#pragma once
#include <vector>
#include <vulkan/vulkan.h>

#include "device.h"

class Swapchain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
    Swapchain(Device& d, VkExtent2D windowExtent);
	~Swapchain();

    bool compareSwapFormat(const Swapchain& swapChain) const { return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat; }
    float extentAspectRatio() const { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }

    VkRenderPass getRenderPass() { return renderPass; }
    VkFramebuffer getFrameBuffer(int index) { return swapchainFrameBuffers[index]; }

    const uint32_t getCurrentFrame() const { return currentFrame; }
    void recreateSwapchain(VkExtent2D newExtent);
    
    VkFormat findDepthFormat();
    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }

private:
    struct oldSwapchainInfo {
        VkSwapchainKHR swapchain;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> swapchainFrameBuffers;
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemories;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
    } oldInfo;
    void copyDataToOldInfo();
    void destroyOldInfo();
    void destroyNonSwapchain();
    void init();
    void createSwapchain(VkSwapchainKHR old = VK_NULL_HANDLE);
    void createImageViews();
    void createRenderPass();
    void createDepthResources();
    void createFramebuffers();
    void createSyncObjects();

    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    Device& device;
    VkExtent2D windowExtent;

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;

    VkFormat swapChainImageFormatOld;
    VkFormat swapChainDepthFormatOld;

    VkExtent2D swapChainExtent;
    VkSwapchainKHR swapchain;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapchainFrameBuffers;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemories;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    uint32_t currentFrame = 0;
};