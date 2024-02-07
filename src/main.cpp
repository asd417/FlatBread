//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>


#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_set>

#include "engine/window.h"
#include "engine/device.h"
#include "App.h"

int main() {
	App app{};
	try {
		app.run();

	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	
    std::cout << "test";
    return 0;
}