#include "Collider.hpp"

#include "Physics.hpp"
#include "Transform.hpp"

void Collider::_createRigidBody(){
	if (_rigidBody) {
		_engine.system<Physics>()._removeFromWorld(_id);
		delete _rigidBody;
	}

	_rigidBody = new btRigidBody(toBt(_mass), _engine.entities.get<Transform>(_id), _compoundShape, toBt(_localInertia));

	_rigidBody->setFriction(toBt(_friction));
	_rigidBody->setRestitution(toBt(_restitution));
	_rigidBody->setSleepingThresholds(toBt(_linearThreshold), toBt(_angularThreshold));
	
	if (hasFlags(_colliderFlags, OverrideGravity))
		_rigidBody->setGravity(toBt(_gravity));

	if (hasFlags(_colliderFlags, OverrideCenterOfMass))
		setCenterOfMass(_centerOfMass);

	if (hasFlags(_colliderFlags, AlwaysActive))
		alwaysActive(true);
}

Collider::Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id), _collisionShape(collisionShape), _mass(mass){}

Collider::~Collider(){
	if (_rigidBody) {
		_engine.system<Physics>()._removeFromWorld(_id);
		delete _rigidBody;
	}

	delete _collisionShape;
}

void Collider::setGravity(const Vec3& direction) {
	if (_rigidBody)
		_rigidBody->setGravity(toBt(direction));
}

void Collider::setFriction(double friction) {
	if (_rigidBody)
		_rigidBody->setFriction(friction);
}

void Collider::setMass(double mass) {

}

void Collider::setDamping(const Vec3 & damping) {

}

void Collider::setRestitution(double restitution){

}

void Collider::setAngularVelocity(const Vec3& velocity) {
	if (_rigidBody)
		_rigidBody->setAngularVelocity(toBt(velocity));
}

void Collider::setLinearVelocity(const Vec3& velocity) {
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

Vec3 Collider::centerOfMass() const {
	return _engine.entities.get<Collider>(_engine.system<Physics>()._rootCollider(_id))->_centerOfMass;
}

void Collider::setCenterOfMass(const Vec3 position) {
	_engine.system<Physics>()._setCenterOfMass(_id, toBt(_centerOfMass));
}
