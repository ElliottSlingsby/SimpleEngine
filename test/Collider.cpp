#include "Collider.hpp"

#include "Transform.hpp"
#include "Physics.hpp"

void Collider::_setWorldPosition() {
	Transform* transform = _engine.entities.get<Transform>(_id);

	if (!_rigidBody) {
		// do special stuff if part of CompoundShape

		return;
	}

	btTransform bulletTransform = _rigidBody->getWorldTransform();
	bulletTransform.setOrigin(toBt(transform->worldPosition()));

	_rigidBody->setWorldTransform(bulletTransform);
}

void Collider::_setWorldRotation() {
	Transform* transform = _engine.entities.get<Transform>(_id);

	if (!_rigidBody) {
		// do special stuff if part of CompoundShape

		return;
	}

	btTransform bulletTransform = _rigidBody->getWorldTransform();
	bulletTransform.setRotation(toBt(transform->worldRotation()));

	_rigidBody->setWorldTransform(bulletTransform);
}

void Collider::_rebuildCompoundShape(){
	Transform* transform = _engine.entities.get<Transform>(_id);

	// find root transform

	// delete CompoundShape using root's pointer

	// create new one and add the root's Shape

	// update root's RigidBody with CompoundSHape

	// recursively go through children and
	//		if has RigidBody remove from world and delete
	//		add their Shape to the CompoundShape
	//		update their CompoundShape pointer
}

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, const btRigidBody::btRigidBodyConstructionInfo& constructionInfo, btDynamicsWorld* dynamicsWorld) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _rigidBody(new btRigidBody(constructionInfo)){
	Transform* transform = _engine.entities.get<Transform>(id);
	transform->_setCollider(this);
	_rigidBody->setUserPointer(transform);
	dynamicsWorld->addRigidBody(_rigidBody);
}

Collider::~Collider(){
	_engine.entities.get<Transform>(_id)->_setCollider(nullptr);

	if (_rigidBody) {
		_engine.system<Physics>()._removeRigidBody(_rigidBody);
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

void Collider::deactivate(){
	if (_rigidBody)
		_rigidBody->forceActivationState(WANTS_DEACTIVATION);
}
