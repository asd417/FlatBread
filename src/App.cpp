#include "App.h"
#include "engine/ecs/entity_components.h"
#include "engine/render_system/spriteRenderSystem.h"
#include "keyboardController.h"

#include <initializer_list>
#include <windows.h>
#include <iostream>
#include <random>
#include <chrono>
//#include<unistd.h> for linux


#define GLOBAL_DESCRIPTOR_COUNT 1000

//ECS::Coordinator is a global singleton for easy access across different parts of the engine
#define ECSCoordiantor ECS::Coordinator::GetCoordinator()

App::App() {
	pool.setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT * GLOBAL_DESCRIPTOR_COUNT);
	pool.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 );
	pool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GLOBAL_DESCRIPTOR_COUNT);
	pool.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GLOBAL_DESCRIPTOR_COUNT);
	pool.Build();

	//DescriptorManager needs to initialized after the pool is done building
	descriptorManager = std::make_unique<DescriptorManager>(device, pool, DESCRIPTOR_COUNT);
	
	createUBO();

	ECSCoordiantor->Init();
	ECSCoordiantor->RegisterComponent<TransformComponent>();

	//Camera setting needs to move into its own class
	setOrthographicProjection(-10, 10, -10, 10, 0, -10);
	setViewDirection({ 0, 0, 0 }, { 0,0,-1 }, { 0,1,0 });
}

void App::run()
{
	auto currentTime = std::chrono::high_resolution_clock::now();

	auto spriteRenderSystem = ECSCoordiantor->RegisterSystem<SpriteRenderSystem>(device, renderer.getSwapchainRenderPass(), descriptorManager->getDescriptorSetLayout());
	ECS::Signature signature;
	signature.set(ECSCoordiantor->GetComponentType<TransformComponent>());
	//signature.set(ecs.GetComponentType<TextureComponent>());
	ECSCoordiantor->SetSystemSignature<SpriteRenderSystem>(signature);

	KeyboardMovementController kCon{};

	std::srand(std::time(nullptr)); // use current time as seed for random generator
	int random_value = std::rand();
	std::cout << "Random value on [0, " << RAND_MAX << "]: " << random_value << "\n";
	Entity removal;
	for (int n = 0; n != 10; ++n)
	{
		int x = 11;
		while (x > 10)
			x = 1 + std::rand() / ((RAND_MAX + 1u) / 10); 

		int y = 11;
		while (y > 10)
			y = 1 + std::rand() / ((RAND_MAX + 1u) / 10);

		int z = 11;
		while (z > 10)
			z = 1 + std::rand() / ((RAND_MAX + 1u) / 10);

		float r = 4;
		while (r > 3)
			r = 1 + std::rand() / ((RAND_MAX + 1u));

		Entity entity = ECSCoordiantor->CreateEntity();
		if (n == 5) removal = entity;
		TransformComponent comp{};
		comp.setTranslation({x-5,y-5});
		comp.setZ(z);
		comp.setRotation(r);
		ECSCoordiantor->AddComponent(entity, comp);
		std::cout << x << " " << y << "\n";
	}

	Entity movingEntity = ECSCoordiantor->CreateEntity();
	TransformComponent comp{};
	comp.setZ(1);
	TransformComponent* added = ECSCoordiantor->AddComponent(movingEntity, comp);

	while (!window.shouldClose()) {
		//Event call function can block therefore we measure the newtime after
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();

		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		// controlling entity with keyboard
		kCon.move(window.window, frameTime, movingEntity);
		if (kCon.pressed(window.window, GLFW_KEY_Q)) {
			ECSCoordiantor->DestroyEntity(removal);
		}

		if (kCon.pressed(window.window, GLFW_KEY_E)) {
			ECSCoordiantor->Serialize();
		}
		if (kCon.pressed(window.window, GLFW_KEY_T)) {
			ECSCoordiantor->Deserialize();
		}


		VkCommandBuffer cmd = renderer.beginPrimaryCMD();
		VkDescriptorSet set = descriptorManager->getDescriptorSet(renderer.getFrameIndex());
		
		UBOstruct ubo{};
		ubo.projectionMatrix = projectionMatrix;
		ubo.viewMatrix = viewMatrix;

		uboBuffer->writeToBuffer(&ubo);
		uboBuffer->flush();

		renderer.beginSwapChainRenderPass(cmd);

		spriteRenderSystem->render(cmd, set);

		renderer.endCurrentRenderPass(cmd);
		renderer.endPrimaryCMD();
	}
}

void App::createUBO()
{
	uboBuffer = std::make_unique<Buffer>(
		device, 
		sizeof(UBOstruct), 
		1, 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	uboBuffer->map();

	descriptorManager->storeUBO(uboBuffer->descriptorInfo());
}

