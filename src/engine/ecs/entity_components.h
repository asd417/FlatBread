#pragma once

#include <glm/glm.hpp>
#include <fstream>
#include <iostream>

struct SerializableComponent {
	virtual void Serialize(std::ofstream& fs) { throw("Serialize NOT IMPLEMENTED"); }
	virtual void Deserialize(std::ifstream& fs) { throw("Deserialize NOT IMPLEMENTED"); }
};

struct TransformComponent : public SerializableComponent{
	TransformComponent() = default;
	TransformComponent(float x, float y, uint32_t z) { translation = { x,y }; zOrder = z; }

	glm::mat3	mat3();

	glm::vec2	getTranslation() const { return translation; }
	void		setTranslation(const glm::vec2& _translation) { translation = _translation; }
	glm::vec2	getScale() const { return scale; }
	void		setScale(const glm::vec2& _scale) { scale = _scale; }
	float		getRotation() const { return rotation; }
	void		setRotation(const float& _rotation) { rotation = _rotation; }
	glm::vec2	getLocalTranslation() const { return localTranslation; }
	void		setLocalTranslation(const glm::vec2& _translation) { localTranslation = _translation; }
	glm::vec2	getLocalScale() const { return localScale; }
	void		setLocalScale(const glm::vec2& _scale) { localScale = _scale; }
	float		getLocalRotation() const { return localRotation; }
	void		setLocalRotation(const float& _rotation) { localRotation = _rotation; }

	uint32_t	getZ() const { return zOrder; }
	void		setZ(const uint32_t& z) { zOrder = z; }

	glm::vec2	getWorldTranslation() const { return translation + localTranslation; };
	glm::vec2	getWorldScale() const { return { scale.x * localScale.x, scale.y * localScale.y }; };
	float		getWorldRotation() const { return rotation + localRotation; };

	//Internally, y cooridnate is flipped.

	void Serialize(std::ofstream& fs) override {
		fs.write( reinterpret_cast<const char*>(&translation.x), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&translation.y), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&scale.x), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&scale.y), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&rotation), sizeof(float));

		fs.write( reinterpret_cast<const char*>(&localTranslation.x), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&localTranslation.y), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&localScale.x), sizeof(float));
		fs.write( reinterpret_cast<const char*>(&localScale.y), sizeof(float));

		fs.write( reinterpret_cast<const char*>(&localRotation), sizeof(float));

		fs.write( reinterpret_cast<const char*>(&zOrder), sizeof(uint32_t));

		std::cout << "Serialized TransformComponent\n";
	}

	void Deserialize(std::ifstream& fs) override {
		fs.read(reinterpret_cast<char*>(&translation.x), sizeof(float));
		fs.read(reinterpret_cast<char*>(&translation.y), sizeof(float));
		fs.read(reinterpret_cast<char*>(&scale.x), sizeof(float));
		fs.read(reinterpret_cast<char*>(&scale.y), sizeof(float));
		fs.read(reinterpret_cast<char*>(&rotation), sizeof(float));

		fs.read(reinterpret_cast<char*>(&localTranslation.x), sizeof(float));
		fs.read(reinterpret_cast<char*>(&localTranslation.y), sizeof(float));
		fs.read(reinterpret_cast<char*>(&localScale.x), sizeof(float));
		fs.read(reinterpret_cast<char*>(&localScale.y), sizeof(float));

		fs.read(reinterpret_cast<char*>(&localRotation), sizeof(float));

		fs.read(reinterpret_cast<char*>(&zOrder), sizeof(uint32_t));
		std::cout << "Deserialized TransformComponent\n";
	}

private:
	glm::vec2 translation = { 0.0f, 0.0f };
	glm::vec2 scale = { 1.0f, 1.0f };
	float rotation = 0.0f;

	glm::vec2 localTranslation = { 0.0f, 0.0f };
	glm::vec2 localScale = { 1.0f, 1.0f };
	float localRotation = 0.0f;

	uint32_t zOrder;
};

struct CameraComponent {
	bool active = false;
};