#pragma once

#include <glm\vec3.hpp>
#include <btBulletDynamicsCommon.h>

struct Collider {
	btCollisionShape* collisionShape = nullptr;
	btRigidBody* rigidBody = nullptr;
};