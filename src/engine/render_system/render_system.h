#include "../buffer.h"
#include "../ecs/entity_component_system.h"

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct PipelineConfigInfo {
	// When removing copy constructors, either implement or default the constructor 
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo operator=(const PipelineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;

	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline {
public:
	Pipeline(std::string vertFilePath, std::string fragFilePath, const PipelineConfigInfo& configInfo);
	~Pipeline();

	// Delete these copy constructors to avoid duplicating the pointers to the vulkan objects
	Pipeline(const Pipeline&) = delete;
	Pipeline operator=(const Pipeline&) = delete;
	void bind(VkCommandBuffer commandBuffer);

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	static void enableAlphaBlending(PipelineConfigInfo& configInfo);

private:
	static std::vector<char> readFile(const std::string& filepath);

	void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

	//shaderModule is a pointer to a pointer
	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	//typedef pointer 
	VkPipeline graphicsPipeline;
	VkPipeline offscreenPipeline;

	// VkShaderModule is a pointer to VkShaderModule_T
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};

class RenderSystem {
protected:
	RenderSystem(
		VkDescriptorSetLayout setLayout,
		size_t pushConstantSize,
		ECS::Coordinator& coordinator);
	~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	void createPipelineLayout(VkDescriptorSetLayout setLayout, size_t pushConstantSize);
	void createPipeline(VkRenderPass renderPass, const char* vertexShaderFilePath, const char* FragmentShaderFilePath, bool enableAlphaBlending, void (*PipelineConfigInfoModifier)(PipelineConfigInfo&));

	ECS::Coordinator& coordinator;
	std::unique_ptr<Pipeline> bglPipeline;
	VkPipelineLayout pipelineLayout;
	std::unique_ptr <Buffer> objDataBuffer;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;

	static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

	bool Vertex::operator=(const Vertex& other) const {
		return std::tie(
			position.x, position.y, position.z)
			== std::tie(other.position.x, other.position.y, other.position.z);
	}
	bool Vertex::operator<(const Vertex& other) const {
		return std::tie(
			position.x, position.y, position.z)
			< std::tie(other.position.x, other.position.y, other.position.z);
	}
	bool Vertex::operator>(const Vertex& other) const {
		return std::tie(
			position.x, position.y, position.z)
	> std::tie(other.position.x, other.position.y, other.position.z);
	}
};