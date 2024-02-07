#include "descriptor_manager.h"
#include "swapchain.h"


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


DescriptorManager::DescriptorManager(Device& d, DescriptorPool& dp, uint32_t descriptorCount) : device{d}, pool{dp}
{
    VkDescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_ALL;
    VkDescriptorSetLayoutBinding storageBinding{};
    storageBinding.binding = 1;
    storageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBinding.descriptorCount = descriptorCount;
    storageBinding.stageFlags = VK_SHADER_STAGE_ALL;
    VkDescriptorSetLayoutBinding imageBinding{};
    imageBinding.binding = 2;
    imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    imageBinding.descriptorCount = descriptorCount;
    imageBinding.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorBindingFlags bindFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    std::array<VkDescriptorBindingFlags, 3> flagsArray = { bindFlags, bindFlags, bindFlags };

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.pNext = nullptr;
    bindingFlags.pBindingFlags = flagsArray.data();
    bindingFlags.bindingCount = 3;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboBinding ,storageBinding,imageBinding };

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 3;
    createInfo.pBindings = bindings.data();
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    createInfo.pNext = &bindingFlags;

    // Create layout
    VK_CHECK(vkCreateDescriptorSetLayout(Device::device(), &createInfo, nullptr, &setLayout));
    // Create Descriptor Sets with the layout, one per swapchain
    for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
        pool.allocateDescriptor(setLayout, descriptorSets[i]);
    }
}


void DescriptorManager::storeUBO(VkDescriptorBufferInfo bufferInfo)
{
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // Write one buffer that is being added
    write.descriptorCount = 1;
    // The array element that we are going to write to
    // is the index, which we refer to as our handles
    write.dstArrayElement = 0;
    write.pBufferInfo = &bufferInfo;

    write.dstBinding = BINDINGS::UNIFORM;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
        write.dstSet = descriptorSets[i];
        vkUpdateDescriptorSets(Device::device(), 1, &write, 0, nullptr);
    }
}

BufferHandle_u32 DescriptorManager::storeBuffer(VkDescriptorBufferInfo bufferInfo, const char* name)
{
    uint32_t handle = buffers.size();
    buffers.push_back(bufferInfo);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.dstBinding = BINDINGS::BUFFER;

    // Write one buffer that is being added
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;
    write.dstArrayElement = handle;

    for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
        write.dstSet = descriptorSets[i];
        vkUpdateDescriptorSets(Device::device(), 1, &write, 0, nullptr);
    }

    if (name != NULL) {
        bufferIndexMap.emplace(std::string(name), handle);
    } else {
        char str1[32] = "BUFFERNAME";
        char str2[16] = "";
        sprintf(str2, "%d", unnamedBufferCount);
        strcat(str1, str2);
        textureIndexMap.emplace(std::string(str1), handle);
        unnamedBufferCount++;
    }
    return handle;
}

TextureHandle_u32 DescriptorManager::storeTexture(VkDescriptorImageInfo imageInfo, VkDeviceMemory memory, VkImage image, const char* name)
{
    uint32_t handle;
    handle = textures.size();
    textures.push_back({ imageInfo, memory, image });

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.dstBinding = BINDINGS::TEXTURE;
    // Write one texture that is being added
    write.descriptorCount = 1;
    // The array element that we are going to write to
    // is the index, which we refer to as our handles
    write.dstArrayElement = handle;
    write.pImageInfo = &imageInfo;

    for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
        write.dstSet = descriptorSets[i];
        vkUpdateDescriptorSets(Device::device(), 1, &write, 0, nullptr);
    }

    if (name != NULL) {
        textureIndexMap.emplace(std::string(name), handle);
    } else {
        char str1[32] = "TEXTURENAME";
        char str2[16] = "";
        sprintf(str2, "%d", unnamedTextureCount);
        strcat(str1, str2);
        textureIndexMap.emplace(std::string(str1), handle);
        unnamedTextureCount++;
    }
    return handle;
}
