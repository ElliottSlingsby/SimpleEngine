#pragma once

#include <btBulletDynamicsCommon.h>

struct Collider {
	btCollisionShape* collisionShape = nullptr;
	btRigidBody* rigidBody = nullptr;
	btMotionState* motionState = nullptr;
};