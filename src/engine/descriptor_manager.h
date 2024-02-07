#pragma once
#include "device.h"
#include "descriptor_pool.h"
#include <array>
#include "swapchain.h"

typedef uint32_t TextureHandle_u32;
typedef uint32_t BufferHandle_u32;

class DescriptorManager {
public:
	enum BINDINGS {
		UNIFORM,
		BUFFER,
		TEXTURE
	};

	struct TexturePackage {
		VkDescriptorImageInfo imageInfo;
		VkDeviceMemory memory;
		VkImage image;
	};

	DescriptorManager(Device& d, DescriptorPool& dp, uint32_t descriptorCount);

	DescriptorManager(const DescriptorManager&) = delete;
	DescriptorManager operator= (const DescriptorManager&) = delete;

	void storeUBO(VkDescriptorBufferInfo bufferInfo);
	BufferHandle_u32 storeBuffer(VkDescriptorBufferInfo bufferInfo, const char* name = NULL);
	TextureHandle_u32 storeTexture(
		VkDescriptorImageInfo imageInfo,
		VkDeviceMemory memory,
		VkImage image,
		const char* name);

	VkDescriptorSetLayout getDescriptorSetLayout() const { return setLayout; }
	VkDescriptorSet getDescriptorSet(int i) const { return descriptorSets[i]; }

private:
	Device& device;
	DescriptorPool& pool;
	VkDescriptorSetLayout setLayout;
	std::array<VkDescriptorSet, Swapchain::MAX_FRAMES_IN_FLIGHT> descriptorSets;

	std::vector<VkDescriptorBufferInfo> buffers{};
	std::vector<TexturePackage> textures{};

	std::unordered_map<std::string, BufferHandle_u32> bufferIndexMap;
	std::unordered_map<std::string, TextureHandle_u32> textureIndexMap;

	uint32_t unnamedTextureCount = 0;
	uint32_t unnamedBufferCount = 0;
};