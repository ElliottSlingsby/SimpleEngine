#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _mass(mass){
	Transform* transform = _engine.entities.add<Transform>(id);

	btVector3 localInertia;

	if (_mass != 0.f)
		_collisionShape->calculateLocalInertia(_mass, localInertia);

	_rigidBody = new btRigidBody(_mass, transform, _collisionShape, localInertia);
	_rigidBody->setUserPointer(transform);
	_engine.system<Physics>()._register(_rigidBody);

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

Collider::~Collider(){
	if (_rigidBody) {
		_engine.system<Physics>()._unregister(_rigidBody);
		delete _rigidBody;
	}
	
	if (_rootCompound)
		delete _compoundShape;

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
