#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, const btRigidBody::btRigidBodyConstructionInfo& constructionInfo) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _rigidBody(constructionInfo){
	Transform* transform = _engine.entities.get<Transform>(id);
	_rigidBody.setUserPointer(transform);

	_engine.system<Physics>()._register(&_rigidBody);
}

Collider::~Collider(){
	_engine.system<Physics>()._unregister(&_rigidBody);

	delete _collisionShape;
}

void Collider::setWorldPosition(const glm::dvec3& position) {
	btTransform worldTransform = _rigidBody.getWorldTransform();
	worldTransform.setOrigin(toBt(position));

	_rigidBody.setWorldTransform(worldTransform);
}

void Collider::setWorldRotation(const glm::dquat& rotation) {
	btTransform worldTransform = _rigidBody.getWorldTransform();
	worldTransform.setRotation(toBt(rotation));

	_rigidBody.setWorldTransform(worldTransform);
}

void Collider::setGravity(const glm::dvec3& direction) {
	_rigidBody.setGravity(toBt(direction));
}

void Collider::setFriction(double friction) {
	_rigidBody.setFriction(friction);
}

void Collider::setAngularVelocity(const glm::dvec3& velocity) {
	_rigidBody.setAngularVelocity(toBt(velocity));
}

void Collider::setLinearVelocity(const glm::dvec3& velocity) {
	_rigidBody.setLinearVelocity(toBt(velocity));
}

void Collider::activate(){
	_rigidBody.activate();
}

void Collider::deactivate(){
	_rigidBody.forceActivationState(WANTS_DEACTIVATION);
}

void Collider::setCompoundInheritance(bool value){
	_compoundInheritance = value;
}
