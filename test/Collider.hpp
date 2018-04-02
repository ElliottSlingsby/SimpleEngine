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
	btRigidBody* _rigidBody;

	//std::vector<btTypedConstraint*> _constraints;

	btCompoundShape* _compoundShape = nullptr;

public:
	void _setWorldPosition();
	void _setWorldRotation();

	void _rebuildCompoundShape();

	Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, const btRigidBody::btRigidBodyConstructionInfo& constructionInfo, btDynamicsWorld* dynamicsWorld);
	~Collider();

	void setGravity(const glm::vec3& direction);
	void setFriction(float friction);

	void setAngularVelocity(const glm::vec3& velocity);
	void setLinearVelocity(const glm::vec3& velocity);

	void activate();
	void deactivate();

	//void ballConstraint(const glm::dvec3& position);
	//void ballConstraint(const glm::dvec3& position, uint64_t id, const glm::dvec3& otherPos);

	//void hingeConstraint(uint64_t id);
	//void sliderConstraint(uint64_t id);

	//friend class Transform;
	//friend class Physics;
};