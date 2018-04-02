#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

#include <vector>

#include "Config.hpp"
#include "Collider.hpp"

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

class Transform : public btMotionState{
	enum InheritanceFlags : uint8_t {
		None = 0x00,
		Position = 0x01,
		Rotation = 0x02,
		Scale = 0x04,
	};

	Engine& _engine;
	const uint64_t _id;

	glm::dvec3 _position;
	glm::dquat _rotation;
	glm::dvec3 _scale = { 1.f, 1.f, 1.f };

	Transform* _parent = nullptr;
	std::vector<Transform*> _children;

	Collider* _collider = nullptr;

	uint8_t _inheritanceFlags = Position | Rotation;

	void getWorldTransform(btTransform& transform) const override;
	void setWorldTransform(const btTransform& transform) override;

	void _setPosition(const glm::dvec3& position);
	void _setRotation(const glm::dquat& rotation);

public:
	void _setCollider(Collider* collider);

	Transform(Engine::EntityManager& entities, uint64_t id);
	~Transform();

	uint64_t id() const;

	void setParent(Transform* other);
	void setChild(Transform* other);

	void removeParent();
	void removeChild(uint32_t i);
	void removeChildren();

	Transform* parent();
	Transform* child(uint32_t i);

	uint32_t children() const;

	void setPosition(const glm::dvec3& position);
	void setRotation(const glm::dquat& rotation);
	void setScale(const glm::dvec3& scale);

	glm::dvec3 position() const;
	glm::dquat rotation() const;
	glm::dvec3 scale() const;

	glm::dvec3 worldPosition() const;
	glm::dquat worldRotation() const;
	glm::dvec3 worldScale() const;

	void rotate(const glm::dquat& rotation);
	void translate(const glm::dvec3& translation);

	void globalRotate(const glm::dquat& rotation);
	void globalTranslate(const glm::dvec3& translation);

	void inheritPosition(bool value);
	void inheritRotation(bool value);
	void inheritScale(bool value);

	//friend class Collider;
};