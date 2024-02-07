#include "render_system.h"

#include "../util.h"

#include <memory>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

Pipeline::Pipeline(std::string vertFilePath, std::string fragFilePath, const PipelineConfigInfo& configInfo)
{
	createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(Device::device(), vertShaderModule, nullptr);
	vkDestroyShaderModule(Device::device(), fragShaderModule, nullptr);
	vkDestroyPipeline(Device::device(), graphicsPipeline, nullptr);
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	// This tells the input assembler to group every three verts into triangles.
	// TRIANGLE_STRIP for example groups every vertex with two previous vertices of the list into a triangle, 
	// meaning all the triangles generated become a single connected strip.
	// This is more memory efficient but it also means that the every triangle is connected unless..
	configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// .. primitiveRestartEnable is true. 
	// if set true while using strip topology, we can insert a special index (0xFFFF or 0xFFFFFFFF) into the index buffer 
	// to break up the strip and start a new one
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	//configInfo.inputAssemblyInfo.
	//VkPipelineViewportStateCreateInfo viewportInfo{};
	// Combine the viewport and the scissor into a 'viewport state create info'
	configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports = nullptr;
	configInfo.viewportInfo.scissorCount = 1;
	configInfo.viewportInfo.pScissors = nullptr;

	// Rasterization stage
	configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If true, z component of the gl_Position will be clamped between 0 and 1
	// Usually set to false because z value less than 0 means the position is behind the camera 
	// while the z value greater than 1 means that the position is too far to render
	// Requires GPU feature to use this
	configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;

	// Discards all primitives before rasterization
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;

	// We can optionally discard triangles depending on their current facing. 
	// This is determined by 'winding order' as in the order of vertex defined.
	// This example sets the winding order of the triangle facing the camera as counter clockwise
	// Backface culling can bring huge performance benefits
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;	//optional
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;				//optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;		//optional

	configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading = 1.0f;
	configInfo.multisampleInfo.pSampleMask = nullptr;
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
	configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

	configInfo.colorBlendAttachment.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f;				//optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f;				//optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f;				//optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f;				//optional

	configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f;
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {};
	configInfo.depthStencilInfo.back = {};

	// Configures pipeline so that it expects viewport and scissor later rather than on initialization
	configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
	configInfo.dynamicStateInfo.flags = 0;
	
	// 2d engine. No vertex. All inputs will be on push constant;
	//configInfo.bindingDescriptions = Vertex::getBindingDescriptions();
	//configInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
}

void Pipeline::enableAlphaBlending(PipelineConfigInfo& configInfo) {
	configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//src is the current value being output from the fragment
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

std::vector<char> Pipeline::readFile(const std::string& filepath)
{
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filepath);
	}
	// instantly gets the file size because the ate bitflag makes the fstream go to the end of the file
	size_t fileSize = static_cast<size_t>(file.tellg());

	// character buffer with a size of the file
	std::vector<char> buffer(fileSize);

	// move to the start of the file
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

void Pipeline::createGraphicsPipeline(
	const std::string& vertFilePath,
	const std::string& fragFilePath,
	const PipelineConfigInfo& configInfo)
{
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");
	auto vertCode = readFile(vertFilePath);
	auto fragCode = readFile(fragFilePath);
	createShaderModule(vertCode, &vertShaderModule);
	createShaderModule(fragCode, &fragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertShaderModule;
	// Name of the entry function of the vertex shader
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragShaderModule;
	// Name of the entry function of the vertex shader
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	auto& bindingDescriptions = configInfo.bindingDescriptions;
	auto& attributeDescriptions = configInfo.attributeDescriptions;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo; //optional. Allows dynamic setting of pipeline configuration

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(Device::device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(Device::device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}
}

RenderSystem::RenderSystem(VkDescriptorSetLayout setLayout, size_t pushConstantSize, ECS::Coordinator& _coordinator) : coordinator{_coordinator}
{
	createPipelineLayout(setLayout, pushConstantSize);
}

RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(Device::device(), pipelineLayout, nullptr);
}

void RenderSystem::createPipelineLayout(VkDescriptorSetLayout setLayout, size_t pushConstantSize)
{
	VkPushConstantRange pushConstantRange{};
	// This flag indicates that we want this pushconstantdata to be accessible in both vertex and fragment shaders
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	// offset mainly for if you want to use separate ranges for vertex and fragment shader
	pushConstantRange.offset = 0;
	pushConstantRange.size = pushConstantSize;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	//desciptor set layout information
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &setLayout;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(Device::device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
}

void RenderSystem::createPipeline(VkRenderPass renderPass, const char* vertexShaderFilePath, const char* fragmentShaderFilePath, bool enableAlphaBlending, void (*PipelineConfigInfoModifier)(PipelineConfigInfo&) = nullptr)
{
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	if (enableAlphaBlending) {
		Pipeline::enableAlphaBlending(pipelineConfig);
	}
	if (PipelineConfigInfoModifier != nullptr) PipelineConfigInfoModifier(pipelineConfig);

	bglPipeline = std::make_unique<Pipeline>(
		util::enginePath(vertexShaderFilePath),
		util::enginePath(fragmentShaderFilePath),
		pipelineConfig);
}


std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
{
	//Possible to write this in brace construction {{0, sizeof(Vertex),VK_VERTEX_INPUT_RATE_VERTEX}}
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex); //This allows easier addition of attribute as the stride will automatically adjusted.
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	//We want both color and vertex in one binding so the binding is kept 0
	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex,position) }); //location, binding, format, offset
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex,color) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT,	offsetof(Vertex,uv) });

	//offsetof macro calculates the byte offset of color member in the Vertex struct

	return attributeDescriptions;
}