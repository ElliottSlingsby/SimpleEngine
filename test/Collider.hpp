#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

#include <glm\vec3.hpp>

class Collider{
	Engine& _engine;
	uint64_t _id;

	btCollisionShape* _collisionShape = nullptr;
	btRigidBody* _rigidBody = nullptr;

public:
	Collider(Engine::EntityManager& entities, uint64_t id);
	~Collider();

	void setGravity(const glm::vec3& direction);
	void setFriction(float friction);

	void setAngularVelocity(const glm::vec3& velocity);
	void setLinearVelocity(const glm::vec3& velocity);

	void activate();

	friend class Transform;
	friend class Physics;
};