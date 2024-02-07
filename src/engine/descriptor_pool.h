#pragma once
#include "device.h"
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>

class DescriptorPool {
public:
	struct PoolBuildInfo {
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		std::vector<VkDescriptorPoolSize> poolSizes;
		VkDescriptorPoolCreateFlags poolFlags;
		uint32_t maxSets;
	};

	DescriptorPool(Device& d, uint32_t descriptorCount);
	~DescriptorPool();

	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator= (const DescriptorPool&) = delete;

	void setMaxSets(uint32_t count) {
		if (buildInfo != nullptr) {
			buildInfo->maxSets = count;
		}
	}

	void addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
		if (buildInfo != nullptr) {
			buildInfo->poolSizes.push_back({ descriptorType, count });
		}
	}

	void addPoolFlags(VkDescriptorPoolCreateFlags flags) {
		if (buildInfo != nullptr) {
			buildInfo->poolFlags = flags;
		}
	}

	void Build();

	bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

private:
	void removeBuildStructure();
	
	Device& device;
	PoolBuildInfo* buildInfo = nullptr;
	VkDescriptorPool descriptorPool;
	uint32_t descriptorCount;
};