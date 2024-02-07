#include "spriteRenderSystem.h"
#include "../ecs/entity_component_system.h"
#include "../ecs/entity_components.h"
#include <iostream>
#include <vulkan/vulkan.h>

struct SpritePushConstant {
	alignas(16) glm::mat4 tMat{ 1.0f };
	glm::vec4 color{};
};

SpriteRenderSystem::SpriteRenderSystem(ECS::Coordinator* c, Device& device, VkRenderPass renderPass, VkDescriptorSetLayout setLayout) : 
	ECS::EntitySystem(),
	RenderSystem{ setLayout, sizeof(SpritePushConstant), *c }
{
	std::cout << "Creating Sprite Render System\n";
	createPipeline(renderPass, "/shaders/simple_shader.vert.spv", "/shaders/simple_shader.frag.spv", true, nullptr);
}


void SpriteRenderSystem::render(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptorSets)
{
	//std::cout << mEntities.size() << " Entities with TransformComponent\n";

	bglPipeline->bind(cmd);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0, //first set
		1, //descriptorSet Count 
		&globalDescriptorSets,
		0, nullptr);

	//vkCmdBindVertexBuffers(cmd, 0, 0, VK_NULL_HANDLE, VK_NULL_HANDLE);

	for (auto& e : mEntities)
	{
		auto& transform = coordinator.GetComponent<TransformComponent>(e);
		//std::cout << transform.getWorldTranslation().x << " " << transform.getWorldTranslation().y << "\n";
		SpritePushConstant push{};
		push.tMat = transform.mat3();
		push.color = glm::vec4{ 128,128,128,1 };

		vkCmdPushConstants(
			cmd,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SpritePushConstant),
			&push);

		vkCmdDraw(cmd, 6, 1, 0, 0);

	}
}
