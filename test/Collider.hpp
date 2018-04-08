#pragma once

#include <btBulletDynamicsCommon.h>

#include "Config.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <vector>

template <typename T>
inline glm::tquat<T> toGlm(const btQuaternion& from) {
	return glm::tquat<T>{
		static_cast<T>(from.w()),
		static_cast<T>(from.x()),
		static_cast<T>(from.y()),
		static_cast<T>(from.z())
	};
}

template <typename T>
inline glm::tvec3<T> toGlm(const btVector3& from) {
	return glm::tvec3<T>{
		static_cast<T>(from.x()),
		static_cast<T>(from.y()),
		static_cast<T>(from.z())
	};
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

class Transform;

class Collider{
	Engine& _engine;
	const uint64_t _id;

	btCollisionShape* const _collisionShape;

	btRigidBody* _rigidBody = nullptr;
	btCompoundShape* _compoundShape = nullptr;

	uint32_t _compoundIndex;

	double _mass = 0.0;

	glm::dvec3 _centerOfMass;

public:
	Collider(Engine::EntityManager& entities, uint64_t id, btCollisionShape* const collisionShape, float mass);
	~Collider();

	void setGravity(const glm::dvec3& direction);
	void setFriction(double friction);

	void setAngularVelocity(const glm::dvec3& velocity);
	void setLinearVelocity(const glm::dvec3& velocity);

	void activate();
	void deactivate();

	void alwaysActive(bool value);

	glm::dvec3 centerOfMass() const;

	void setCenterOfMass(const glm::dvec3 position);

	friend class Physics;
};