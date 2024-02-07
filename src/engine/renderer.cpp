#include "renderer.h"
#include <cassert>

#define VK_CHECK(x)                                                     \
	do                                                                  \
	{                                                                   \
		VkResult err = x;                                               \
		if (err)                                                        \
		{                                                               \
			std::cout <<"Detected Vulkan error: " << err << std::endl;  \
			abort();                                                    \
		}                                                               \
	} while (0)

Renderer::Renderer(Window& w, Device& d) : window{ w }, device{ d }
{
	recreateSwapChain();
	createCommandBuffers();
}

VkCommandBuffer Renderer::beginPrimaryCMD()
{
	assert(!isFrameStarted && "Can not call beginPrimaryCMD() while the frame is already started");
	auto result = swapchain->acquireNextImage(&currentImageIndex);

	//detect if the surface is no longer compatible with the swapchain
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return nullptr;
	}

	if (result != VK_SUCCESS && VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	isFrameStarted = true;
	auto commandBuffer = getCurrentCommandBuffer();

	// Record draw commands to each command buffers
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording draw command to command buffers");
	}
	return commandBuffer;
}

void Renderer::endPrimaryCMD()
{
	assert(isFrameStarted && "Cannot call endFrame() while frame is not in progress");

	auto commandBuffer = getCurrentCommandBuffer();
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer");
	}

	auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
	//VK_SUBOPTIMAL_KHR means the swapchain no longer matches the surface but can be used to present on the surface
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized())
	{
		window.resetWindowResizedFlag();
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swapchain image");
	}

	isFrameStarted = false;
	currentImageIndex = (currentImageIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Cannot call beginSwapChainRenderPass() while frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin renderpass from a different frame");

	// Record draw commands to each command buffers
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

	renderPassInfo.renderPass = swapchain->getRenderPass();
	renderPassInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);

	renderPassInfo.renderArea.offset = { 0,0 };
	// Make sure to use the swapchain extent not the window extent
	// because the swapchain extent may be larger then window extent which is the case in Mac retina display
	renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

	// Set the color that the frame buffer 'attachments' will clear to 
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // color attachment
	clearValues[1].depthStencil = { 1.0f, 0 }; // Depths stencil clear value

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	// VK_SUBPASS_CONTENTS_INLINE indicate that no secondary command buffers are in use

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0,0}, swapchain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endCurrentRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Cannot call endSwapChainRenderPass() while frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end renderpass from a different frame");
	vkCmdEndRenderPass(commandBuffer);
}

void Renderer::recreateSwapChain()
{
	//Get the new window extent
	VkExtent2D extent = window.getExtent();
	//If there is at least one dimension with length of 0(or sizeless), the program will pause which is during minimization.
	while (extent.width == 0 || extent.height == 0) {
		extent = window.getExtent();
		glfwWaitEvents();
	}
	//Wait for current swapchain to no longer be used
	vkDeviceWaitIdle(Device::device());
	if (swapchain == nullptr) {
		swapchain = std::make_unique<Swapchain>(device, extent);
	}
	else {
		swapchain->recreateSwapchain(extent);
	}
	//Pipeline relies on swapchain so the createPipeline() must follow after creating a new swapchain
	// if render pass is compatible do nothing else
	
}


void Renderer::createCommandBuffers()
{
	commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	// Primary command buffers can be submitted to device graphics queue for execution but can not be called by other command buffers
	// Secondary command buffers can not be submitted to the queue but can be called by other command buffers

	allocInfo.commandPool = device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t> (commandBuffers.size());

	if (vkAllocateCommandBuffers(Device::device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to Allocate Command Buffers");
	}
}


void Renderer::setUpOffScreenRenderPass(uint32_t textureWidth, uint32_t textureHeight)
{
	createOffScreenRenderPass(textureWidth, textureHeight);
	createOffscreenFrameBuffer();
}

void Renderer::createOffScreenRenderPass(uint32_t textureWidth, uint32_t textureHeight)
{
	offscreenPass.width = textureWidth;
	offscreenPass.height = textureHeight;
	// Depth stencil attachment
	VkFormat fbDepthFormat;
	VkBool32 validDepthFormat = device.getSupportedDepthsFormat(&fbDepthFormat);
	assert(validDepthFormat);
	createOffscreenColorAttachment();
	createOffscreenDepthsAttachment(fbDepthFormat);

	std::array<VkAttachmentDescription, 2> attchmentDescriptions{};
	createOffscreenAttachmentDescriptors(attchmentDescriptions, fbDepthFormat);

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;

	std::array<VkSubpassDependency, 2> dependencies{};
	createOffscreenSubpassDependencies(dependencies);

	// Create the actual renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
	renderPassInfo.pAttachments = attchmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK(vkCreateRenderPass(Device::device(), &renderPassInfo, nullptr, &offscreenPass.renderPass));

}

void Renderer::createOffscreenColorAttachment()
{
	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
	image.extent.width = offscreenPass.width;
	image.extent.height = offscreenPass.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VK_CHECK(vkCreateImage(Device::device(), &image, nullptr, &offscreenPass.color.image));
	vkGetImageMemoryRequirements(Device::device(), offscreenPass.color.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(Device::device(), &memAlloc, nullptr, &offscreenPass.color.mem));
	VK_CHECK(vkBindImageMemory(Device::device(), offscreenPass.color.image, offscreenPass.color.mem, 0));

	VkImageViewCreateInfo colorImageView{};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = offscreenPass.color.image;
	VK_CHECK(vkCreateImageView(Device::device(), &colorImageView, nullptr, &offscreenPass.color.view));

	// Create sampler to sample from the attachment in the fragment shader
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = samplerInfo.addressModeU;
	samplerInfo.addressModeW = samplerInfo.addressModeU;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK(vkCreateSampler(Device::device(), &samplerInfo, nullptr, &offscreenPass.sampler));
}

void Renderer::createOffscreenDepthsAttachment(VkFormat& depthsFormat)
{
	VkImageCreateInfo image{};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = depthsFormat;
	image.extent.width = offscreenPass.width;
	image.extent.height = offscreenPass.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VK_CHECK(vkCreateImage(Device::device(), &image, nullptr, &offscreenPass.depth.image));
	vkGetImageMemoryRequirements(Device::device(), offscreenPass.depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(Device::device(), &memAlloc, nullptr, &offscreenPass.depth.mem));
	VK_CHECK(vkBindImageMemory(Device::device(), offscreenPass.depth.image, offscreenPass.depth.mem, 0));

	VkImageViewCreateInfo depthStencilView{};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = depthsFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (depthsFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = offscreenPass.depth.image;
	VK_CHECK(vkCreateImageView(Device::device(), &depthStencilView, nullptr, &offscreenPass.depth.view));
}

void Renderer::createOffscreenAttachmentDescriptors(std::array<VkAttachmentDescription, 2>& descriptors, VkFormat& depthsFormat)
{
	// Color attachment
	descriptors[0].format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
	descriptors[0].samples = VK_SAMPLE_COUNT_1_BIT;
	descriptors[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	descriptors[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	descriptors[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	descriptors[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	descriptors[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	descriptors[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// Depth attachment
	descriptors[1].format = depthsFormat;
	descriptors[1].samples = VK_SAMPLE_COUNT_1_BIT;
	descriptors[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	descriptors[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	descriptors[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	descriptors[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	descriptors[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	descriptors[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
}

void Renderer::createOffscreenSubpassDependencies(std::array<VkSubpassDependency, 2>& dependencies)
{
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
}

void Renderer::createOffscreenFrameBuffer()
{
	VkImageView attachments[2];
	attachments[0] = offscreenPass.color.view;
	attachments[1] = offscreenPass.depth.view;

	VkFramebufferCreateInfo fbufCreateInfo{};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = offscreenPass.renderPass;
	fbufCreateInfo.attachmentCount = 2;
	fbufCreateInfo.pAttachments = attachments;
	fbufCreateInfo.width = offscreenPass.width;
	fbufCreateInfo.height = offscreenPass.height;
	fbufCreateInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(Device::device(), &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer));
}


