#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <vector>

class Transform;

class Collider{
	Engine& _engine;
	const uint64_t _id;

	float _mass;

	btCollisionShape* const _collisionShape;
	btRigidBody* _rigidBody;

	btCompoundShape* _compoundShape = nullptr;
	bool _rootCompound = false;
	uint32_t _compoundIndex;

public:
	Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass);
	~Collider();

	void setGravity(const glm::dvec3& direction);
	void setFriction(double friction);

	void setAngularVelocity(const glm::dvec3& velocity);
	void setLinearVelocity(const glm::dvec3& velocity);

	void activate();
	void deactivate();

	void alwaysActive(bool value);

	friend class Physics;
};