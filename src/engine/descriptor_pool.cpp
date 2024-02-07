#include "descriptor_pool.h"
#include <cassert>
#include <vector>

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


DescriptorPool::DescriptorPool(Device& d, uint32_t _descriptorCount) : device{d}
{
    buildInfo = new PoolBuildInfo();
    descriptorCount = _descriptorCount;
    descriptorPool = VK_NULL_HANDLE;
}

DescriptorPool::~DescriptorPool()
{
    removeBuildStructure();
}

void DescriptorPool::Build()
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(buildInfo->poolSizes.size());
    descriptorPoolInfo.pPoolSizes = buildInfo->poolSizes.data();
    descriptorPoolInfo.maxSets = buildInfo->maxSets;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT | buildInfo->poolFlags;

    std::cout << "Building Descriptor Pool with poolSizeCount: " << descriptorPoolInfo.poolSizeCount << "\n";
    for (auto& p : buildInfo->poolSizes) {
        std::cout << "  type: " << p.type << "\n";
        std::cout << "  descriptorCount: " << p.descriptorCount << "\n";
    }

    if (vkCreateDescriptorPool(Device::device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    removeBuildStructure();
}

bool DescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
    uint32_t max_binding = descriptorCount;
    countInfo.descriptorSetCount = 1;
    countInfo.pDescriptorCounts = &max_binding;
    allocInfo.pNext = &countInfo;

    VK_CHECK(vkAllocateDescriptorSets(Device::device(), &allocInfo, &descriptor));
    return true;
}

void DescriptorPool::removeBuildStructure() {
    if (buildInfo == nullptr) return;
    delete buildInfo;
    buildInfo = nullptr;
}
