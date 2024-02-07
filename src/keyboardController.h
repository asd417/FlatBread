#include "engine/window.h"
#include "engine/ecs/entity_component_system.h"
#include "engine/ecs/entity_components.h"
class KeyboardMovementController {
public:
	struct KeyMappings {
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveUp = GLFW_KEY_W;
		int moveDown = GLFW_KEY_S;
	};

	//edits value of the TransformComponent of the entity to move
	bool move(GLFWwindow* window, float dt, Entity ent);
	bool pressed(GLFWwindow* window, int GLFW_KEY);

	KeyMappings keys{};
	float moveSpeed{ 5.0f };
};