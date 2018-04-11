#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <vector>

inline Quat fromBt(const btQuaternion& from) {
	return Quat{
		static_cast<Number>(from.w()),
		static_cast<Number>(from.x()),
		static_cast<Number>(from.y()),
		static_cast<Number>(from.z())
	};
}

inline Vec3 fromBt(const btVector3& from) {
	return Vec3{
		static_cast<Number>(from.x()),
		static_cast<Number>(from.y()),
		static_cast<Number>(from.z())
	};
}

inline Number fromBt(const btScalar& from) {
	return static_cast<Number>(from);
}

inline btQuaternion toBt(const Quat& from) {
	return btQuaternion{
		static_cast<btScalar>(from.x),
		static_cast<btScalar>(from.y),
		static_cast<btScalar>(from.z),
		static_cast<btScalar>(from.w)
	};
}

inline btVector3 toBt(const Vec3& from) {
	return btVector3{
		static_cast<btScalar>(from.x),
		static_cast<btScalar>(from.y),
		static_cast<btScalar>(from.z)
	};
}

inline btScalar toBt(const Number& from) {
	return static_cast<btScalar>(from);
}

class Collider{
	enum PhysicsFlags : uint8_t {
		None = 0,

		AlwaysActive = 1,
		OverrideGravity = 2,
		OverrideCenterOfMass = 4
	};

	Engine& _engine;
	const uint64_t _id;

	btCollisionShape* const _collisionShape;

	btRigidBody* _rigidBody = nullptr;
	btCompoundShape* _compoundShape = nullptr;

	uint32_t _compoundIndex;

	double _mass;

	Vec3 _gravity;
	Vec3 _centerOfMass;
	Vec3 _localInertia;

	double _friction = 0.5;
	double _restitution = 0.0;
	double _linearThreshold = 1.0;
	double _angularThreshold = 0.8;

	uint8_t _colliderFlags = None;

	void _createRigidBody();

public:
	Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass);
	~Collider();

	void setGravity(const Vec3& direction);
	void setFriction(double friction);
	void setMass(double mass);
	void setDamping(const Vec3& damping);
	void setRestitution(double restitution);

	void setAngularVelocity(const Vec3& velocity);
	void setLinearVelocity(const Vec3& velocity);

	void activate();
	void deactivate();

	void alwaysActive(bool value);

	Vec3 centerOfMass() const;

	void setCenterOfMass(const Vec3 position);

	/*
	void setSleepingThresholds(double linear, double angular);

	void applyCentralForce(const Vec3& force);
	void applyTorque(const Vec3& torque);
	void applyForce(const Vec3& force, const Vec3& position);
	void applyCentralImpulse(const Vec3& impulse);
	void applyTorqueImpulse(const Vec3& torque);
	void applyImpulse(const Vec3& impulse, const Vec3& position);

	Vec3 totalForce() const;
	Vec3 totalTorque() const;

	void clearForces();
	*/

	friend class Physics;
};