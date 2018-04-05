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

	btCollisionShape* const _collisionShape;

	btRigidBody* _rigidBody = nullptr;
	btCompoundShape* _compoundShape = nullptr;

	uint32_t _compoundIndex;

	double _mass = 0.0;

public:
	glm::dvec3 _centerOfMass;

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