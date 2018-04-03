#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, const btRigidBody::btRigidBodyConstructionInfo& constructionInfo) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _constructionInfo(constructionInfo), _rigidBody(new btRigidBody(constructionInfo)){
	Transform* transform = _engine.entities.get<Transform>(id);
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

//void Collider::setCompoundInheritance(bool value){
//	_compoundInheritance = value;
//}
