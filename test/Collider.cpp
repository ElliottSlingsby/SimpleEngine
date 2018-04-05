#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _mass(mass){}

Collider::~Collider(){
	if (_rigidBody) {
		_engine.system<Physics>()._removeFromWorld(_id);
		delete _rigidBody;
	}

	delete _collisionShape;
}

void Collider::setGravity(const glm::dvec3& direction) {
	if (_rigidBody)
		_rigidBody->setGravity(toBt(direction));
}

void Collider::setFriction(double friction) {
	if (_rigidBody)
		_rigidBody->setFriction(friction);
}

void Collider::setAngularVelocity(const glm::dvec3& velocity) {
	if (_rigidBody)
		_rigidBody->setAngularVelocity(toBt(velocity));
}

void Collider::setLinearVelocity(const glm::dvec3& velocity) {
	if (_rigidBody)
		_rigidBody->setLinearVelocity(toBt(velocity));
}

void Collider::activate(){
	if (_rigidBody)
		_rigidBody->activate();
}

void Collider::deactivate(){
	if (_rigidBody)
		_rigidBody->forceActivationState(WANTS_DEACTIVATION);
}

void Collider::alwaysActive(bool value){
	if (_rigidBody)
		_rigidBody->forceActivationState(DISABLE_DEACTIVATION);
}
