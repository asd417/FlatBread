#pragma once
#include "../ecs/entity_component_system.h"
#include "render_system.h"
#include "../device.h"

class SpriteRenderSystem : public ECS::EntitySystem, public RenderSystem {
public:
	SpriteRenderSystem(ECS::Coordinator* c, Device& device, VkRenderPass renderPass, VkDescriptorSetLayout setLayout);
	SpriteRenderSystem::~SpriteRenderSystem() = default;

	SpriteRenderSystem(const SpriteRenderSystem&) = delete;
	SpriteRenderSystem& operator=(const SpriteRenderSystem&) = delete;

	void render(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptorSets);

private:
    
};