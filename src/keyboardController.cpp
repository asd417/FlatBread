#include "keyboardController.h"
#include <iostream>

#define ECSCoordiantor ECS::Coordinator::GetCoordinator()

bool KeyboardMovementController::move(GLFWwindow* window, float dt, Entity ent)
{
	bool updated = false;
	glm::vec2 moveDir{ 0.0f };
	
	if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += glm::vec2(1, 0);
	if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= glm::vec2(1, 0);
	if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += glm::vec2(0, 1);
	if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= glm::vec2(0, 1);
	
	TransformComponent& tc = ECSCoordiantor->GetComponent<TransformComponent>(ent);

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
		//std::cout << "TC pointer:" << &tc << "\n";
		tc.setTranslation(tc.getWorldTranslation() + moveSpeed * dt * glm::normalize(moveDir));
		updated = true;
		//std::cout << gameObject.transform.translation[transformIndex].x << " " << gameObject.transform.translation[transformIndex].y << " " << gameObject.transform.translation[transformIndex].z << "\n";
	}
    return false;
}

bool KeyboardMovementController::pressed(GLFWwindow* window, int GLFW_KEY)
{
	return glfwGetKey(window, GLFW_KEY) == GLFW_PRESS;
}

