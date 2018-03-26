#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id) {
	if (_engine.entities.has<Transform>(_id))
		_engine.entities.get<Transform>(id)->_collider = this;
}

Collider::~Collider(){
	if (_engine.entities.has<Transform>(_id))
		_engine.entities.get<Transform>(_id)->_collider = nullptr;

	if (_rigidBody) {
		_engine.system<Physics>()._dynamicsWorld->removeRigidBody(_rigidBody);
		delete _rigidBody;
	}

	if (_collisionShape)
		delete _collisionShape;
}

void Collider::setGravity(const glm::vec3& direction) {
	if (_rigidBody)
		_rigidBody->setGravity(btVector3(direction.x, direction.y, direction.z));
}

void Collider::setFriction(float friction) {
	if (_rigidBody)
		_rigidBody->setFriction(friction);
}

void Collider::setAngularVelocity(const glm::vec3& velocity) {
	if (_rigidBody)
		_rigidBody->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}

void Collider::setLinearVelocity(const glm::vec3& velocity) {
	if (_rigidBody)
		_rigidBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}

void Collider::activate(){
	if (_rigidBody)
		_rigidBody->activate();
}
