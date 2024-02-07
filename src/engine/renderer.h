#pragma once
#include "device.h"
#include "window.h"
#include "swapchain.h"

#include <vector>
#include <array>

class Renderer {
public:
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};
	Renderer(Window& w, Device& device);
	~Renderer() = default;

	Renderer(const Renderer&) = delete;
	Renderer& operator= (const Renderer&) = delete;

	VkRenderPass getSwapchainRenderPass() const { return swapchain->getRenderPass();}

	VkCommandBuffer beginPrimaryCMD();
	void endPrimaryCMD();

	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endCurrentRenderPass(VkCommandBuffer commandBuffer);

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
		return commandBuffers[currentImageIndex];
	}
	int getFrameIndex() const {
		assert(isFrameStarted && "Cannot get frame index when frame not in progress");
		return currentImageIndex;
	}

	struct ExtraRenderPass {
		uint32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment color;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
		VkSampler sampler;
		uint32_t renderTargetHandle;
		VkDescriptorImageInfo colorImageInfo;
	};
private:
	void createCommandBuffers();
	void recreateSwapChain();

	ExtraRenderPass offscreenPass{};

	void setUpOffScreenRenderPass(uint32_t textureWidth, uint32_t textureHeight);

	void createOffScreenRenderPass(uint32_t textureWidth, uint32_t textureHeight);
	void createOffscreenColorAttachment();

	void createOffscreenDepthsAttachment(VkFormat& depthsFormat);
	void createOffscreenAttachmentDescriptors(std::array<VkAttachmentDescription, 2>& descriptors, VkFormat& depthsFormat);
	void createOffscreenSubpassDependencies(std::array<VkSubpassDependency, 2>& dependencies);
	void createOffscreenFrameBuffer();
	Window& window;
	Device& device;
	std::unique_ptr<Swapchain> swapchain;
	std::vector<VkCommandBuffer> commandBuffers;
	bool frameStarted = false;

	uint32_t currentImageIndex = 0;
	bool isFrameStarted = false;


};