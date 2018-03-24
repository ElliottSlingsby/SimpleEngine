#pragma once

#include <glm\vec3.hpp>
#include <btBulletDynamicsCommon.h>

struct Collider {
	btCollisionShape* collisionShape = nullptr;
	btRigidBody* rigidBody = nullptr;

	void impulse(const glm::vec3& velocity) {
		rigidBody->applyCentralImpulse(btVector3(velocity.x, velocity.y, velocity.z));
	}

	void torque(const glm::vec3& rotation) {
		rigidBody->applyTorqueImpulse(btVector3(rotation.x, rotation.y, rotation.z));
	}
};