#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

#include <vector>

#include "Config.hpp"
#include "Collider.hpp"

class Transform : public btMotionState{
	Engine& _engine;
	uint64_t _id;

	Transform* _parent = nullptr;
	std::vector<Transform*> _children;

	Collider* _collider = nullptr;

	glm::dvec3 _position;
	glm::dquat _rotation;
	glm::dvec3 _scale = { 1.f, 1.f, 1.f };

	void _setPosition(const glm::dvec3& position);
	void _setRotation(const glm::dquat& rotation);

public:
	Transform(Engine::EntityManager& entities, uint64_t id);
	~Transform();

	void setParent(Transform* other);
	void setChild(Transform* other);

	void removeParent();
	void removeChildren();

	void setPosition(const glm::dvec3& position);
	void setRotation(const glm::dquat& rotation);
	void setScale(const glm::dvec3& scale);

	glm::dvec3 position() const;
	glm::dquat rotation() const;
	glm::dvec3 scale() const;

	glm::dvec3 worldPosition() const;
	glm::dquat worldRotation() const;
	glm::dvec3 worldScale() const;

	void rotate(const glm::dquat& rotation);
	void translate(const glm::dvec3& translation);

	void globalRotate(const glm::dquat& rotation);
	void globalTranslate(const glm::dvec3& translation);

	void getWorldTransform(btTransform& transform) const override;
	void setWorldTransform(const btTransform& transform) override;

	friend class Collider;
};