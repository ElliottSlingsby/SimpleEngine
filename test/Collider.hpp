#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

class Collider : Engine::Component{
public:
	btCollisionShape* collisionShape = nullptr;
	btRigidBody* rigidBody = nullptr;

	Collider(Engine::EntityManager& entities, uint64_t id);
	~Collider();
};