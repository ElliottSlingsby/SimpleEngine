#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

#include <vector>

#include "Config.hpp"
#include "Collider.hpp"
#include "RenderCoords.hpp"

class Transform : public btMotionState{
	Engine& _engine;
	const uint64_t _id;

	glm::dvec3 _position;
	glm::dquat _rotation;
	glm::dvec3 _scale = { 1.f, 1.f, 1.f };

	glm::dvec3 _offsetPosition;

	uint64_t _parent = 0;
	std::vector<uint64_t> _children;

	void getWorldTransform(btTransform& transform) const override;
	void setWorldTransform(const btTransform& transform) override;

	void _setPosition(const glm::dvec3& position);
	void _setRotation(const glm::dquat& rotation);
	void _setScale(const glm::dvec3& scale);

	void _makeGlobal();
	void _makeLocal(Transform* transform);

public:
	Transform(Engine::EntityManager& entities, uint64_t id);
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

	void setPosition(const glm::dvec3& position);
	void setRotation(const glm::dquat& rotation);
	void setScale(const glm::dvec3& scale);

	void setWorldPosition(const glm::dvec3& position);
	void setWorldRotation(const glm::dquat& rotation);
	void setWorldScale(const glm::dvec3& scale);

	glm::dvec3 position() const;
	glm::dquat rotation() const;
	glm::dvec3 scale() const;

	glm::dvec3 worldPosition() const;
	glm::dquat worldRotation() const;
	glm::dvec3 worldScale() const;

	glm::dmat4 matrix() const;

	void rotate(const glm::dquat& rotation);
	void translate(const glm::dvec3& translation);
	void scale(const glm::dvec3& scaling);

	void worldRotate(const glm::dquat& rotation);
	void worldTranslate(const glm::dvec3& translation);
	void worldScale(const glm::dvec3& scaling);

	void lookAt(glm::dvec3& position, glm::dvec3 up = GlobalDVec3::up);
};