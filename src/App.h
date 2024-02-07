#pragma once
#include "engine/device.h"
#include "engine/window.h"
#include "engine/descriptor_pool.h"
#include "engine/renderer.h"
#include "engine/descriptor_manager.h"
#include "engine/buffer.h"
#include "engine/ecs/entity_component_system.h"

#include <glm/glm.hpp>

//Max number of texture / buffer bound.
#define DESCRIPTOR_COUNT 1000

class App {
public:
	struct UBOstruct {
        glm::mat4 projectionMatrix{ 1.f };
        glm::mat4 viewMatrix{ 1.f };
	};
	App();
	~App() {
        ECS::Coordinator::DeleteCoordinator();
    };
	void run();
	void createUBO();
private:
	Window window{ 800, 800, "Flatbread" };
	Device device{ window };
	DescriptorPool pool{ device, DESCRIPTOR_COUNT };
	std::unique_ptr<DescriptorManager> descriptorManager;
	Renderer renderer{ window, device };
	std::unique_ptr<Buffer> uboBuffer;
	//ECS::Coordinator ecs{};

	//Camera
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
    {
        glm::mat4 pTransformMat = glm::mat4{ 1.0f };
        pTransformMat[0][0] = 2.f / (right - left);
        pTransformMat[1][1] = 2.f / (bottom - top);
        pTransformMat[2][2] = 1.f / (far - near);
        pTransformMat[3][0] = -(right + left) / (right - left);
        pTransformMat[3][1] = -(bottom + top) / (bottom - top);
        pTransformMat[3][2] = -near / (far - near);
        projectionMatrix = pTransformMat;
    }

    void setViewInverseView(glm::vec3 position, glm::vec3 u, glm::vec3 v, glm::vec3 w)
    {
        viewMatrix = glm::mat4{ 1.f };
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        // construct orthonormal basis
        // three unit vectors 
        const glm::vec3 w{ glm::normalize(direction) };
        const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
        const glm::vec3 v{ glm::cross(w, u) };
        setViewInverseView(position, u, v, w);
    }

};