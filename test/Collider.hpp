#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <vector>

template <typename T>
inline glm::tquat<T> fromBt(const btQuaternion& from) {
	return glm::tquat<T>{
		static_cast<T>(from.w()),
		static_cast<T>(from.x()),
		static_cast<T>(from.y()),
		static_cast<T>(from.z())
	};
}

template <typename T>
inline glm::tvec3<T> fromBt(const btVector3& from) {
	return glm::tvec3<T>{
		static_cast<T>(from.x()),
		static_cast<T>(from.y()),
		static_cast<T>(from.z())
	};
}

template <typename T>
inline T fromBt(const btScalar& from) {
	return static_cast<T>(from);
}

template <typename T>
inline btQuaternion toBt(const glm::tquat<T>& from) {
	return btQuaternion{
		static_cast<btScalar>(from.x),
		static_cast<btScalar>(from.y),
		static_cast<btScalar>(from.z),
		static_cast<btScalar>(from.w)
	};
}

template <typename T>
inline btVector3 toBt(const glm::tvec3<T>& from) {
	return btVector3{
		static_cast<btScalar>(from.x),
		static_cast<btScalar>(from.y),
		static_cast<btScalar>(from.z)
	};
}

template <typename T>
inline btScalar toBt(const T& from) {
	return static_cast<btScalar>(from);
}

class Transform;

class Collider{
	enum PhysicsFlags : uint8_t {
		None = 0,

		FormCompoundShape = 1,
		UpdateCenterOfMass = 2,
		AlwaysActive = 4,
		OverrideGravity = 8,
		OverrideCenterOfMass = 16
	};

	Engine& _engine;
	const uint64_t _id;

	btCollisionShape* const _collisionShape;

	btRigidBody* _rigidBody = nullptr;
	btCompoundShape* _compoundShape = nullptr;

	uint32_t _compoundIndex;

	double _mass;

	glm::dvec3 _gravity;
	glm::dvec3 _centerOfMass;
	glm::dvec3 _localInertia;

	double _friction = 0.5;
	double _restitution = 0.0;
	double _linearThreshold = 1.0;
	double _angularThreshold = 0.8;

	uint8_t _colliderFlags = None;

	void _createRigidBody();

public:
	Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass);
	~Collider();

	void setGravity(const glm::dvec3& direction);
	void setFriction(double friction);
	void setMass(double mass);
	void setDamping(const glm::dvec3& damping);
	void setRestitution(double restitution);

	void setAngularVelocity(const glm::dvec3& velocity);
	void setLinearVelocity(const glm::dvec3& velocity);

	void activate();
	void deactivate();

	void alwaysActive(bool value);

	glm::dvec3 centerOfMass() const;

	void setCenterOfMass(const glm::dvec3 position);

	/*
	void setSleepingThresholds(double linear, double angular);

	void applyCentralForce(const glm::dvec3& force);
	void applyTorque(const glm::dvec3& torque);
	void applyForce(const glm::dvec3& force, const glm::dvec3& position);
	void applyCentralImpulse(const glm::dvec3& impulse);
	void applyTorqueImpulse(const glm::dvec3& torque);
	void applyImpulse(const glm::dvec3& impulse, const glm::dvec3& position);

	glm::dvec3 totalForce() const;
	glm::dvec3 totalTorque() const;

	void clearForces();
	*/

	friend class Physics;
};