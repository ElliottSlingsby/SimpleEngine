#pragma once

#include "SystemInterface.hpp"

#include <vector>

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

namespace WorldVec3 {
	const glm::vec3 up(0, 0, 1);
	const glm::vec3 down(0, 0, -1);
	const glm::vec3 left(-1, 0, 0);
	const glm::vec3 right(1, 0, 0);
	const glm::vec3 forward(0, 1, 0);
	const glm::vec3 back(0, -1, 0);
}

namespace LocalVec3 {
	const glm::vec3 up(0, 1, 0);
	const glm::vec3 down(0, -1, 0);
	const glm::vec3 left(-1, 0, 0);
	const glm::vec3 right(1, 0, 0);
	const glm::vec3 forward(0, 0, -1);
	const glm::vec3 back(0, 0, 1);
}

class Transform{
	SystemInterface::Engine& _engine;
	const uint64_t _id;

	glm::vec3 _position;
	glm::quat _rotation;
	glm::vec3 _scale = { 1, 1, 1 };

	uint64_t _parent = 0;
	std::vector<uint64_t> _children; // maybe uint64_t _children[maxChildren]; ???

	void _setPosition(const glm::vec3& position);
	void _setRotation(const glm::quat& rotation);
	void _setScale(const glm::vec3& scale);

	void _makeGlobal();
	void _makeLocal(Transform* transform);

public:
	Transform(SystemInterface::Engine& engine, uint64_t id);
	~Transform();

	uint64_t id() const;

	void setParent(uint64_t other, bool localize = true);
	void setChild(uint64_t other, bool localize = true);

	void removeParent(bool globalize = true);
	void removeChild(uint32_t i, bool globalize = true);
	void removeChildren(bool globalize = true);

	uint64_t root() const;

	uint64_t parent() const;
	uint64_t child(uint32_t i) const;
	uint32_t children() const;

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::quat& rotation);
	void setScale(const glm::vec3& scale);

	void setWorldPosition(const glm::vec3& position);
	void setWorldRotation(const glm::quat& rotation);
	void setWorldScale(const glm::vec3& scale);

	glm::vec3 position() const;
	glm::quat rotation() const;
	glm::vec3 scale() const;

	glm::vec3 worldPosition() const;
	glm::quat worldRotation() const;
	glm::vec3 worldScale() const;

	glm::mat4 matrix() const;

	void rotate(const glm::quat& rotation);
	void translate(const glm::vec3& translation);
	void scale(const glm::vec3& scaling);

	void worldRotate(const glm::quat& rotation);
	void worldTranslate(const glm::vec3& translation);
	void worldScale(const glm::vec3& scaling);

	void lookAt(glm::vec3& position, glm::vec3 up = WorldVec3::up);
};