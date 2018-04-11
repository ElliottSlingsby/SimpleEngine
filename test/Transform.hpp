#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

#include <vector>

#include "Config.hpp"
#include "Collider.hpp"

class Transform : public btMotionState{
	Engine& _engine;
	const uint64_t _id;

	Vec3 _position;
	Quat _rotation;
	Vec3 _scale = { 1.f, 1.f, 1.f };

	Vec3 _offsetPosition;

	uint64_t _parent = 0;
	std::vector<uint64_t> _children;

	void getWorldTransform(btTransform& transform) const override;
	void setWorldTransform(const btTransform& transform) override;

	void _setPosition(const Vec3& position);
	void _setRotation(const Quat& rotation);
	void _setScale(const Vec3& scale);

	void _makeGlobal();
	void _makeLocal(Transform* transform);

public:
	Transform(Engine::EntityManager& entities, uint64_t id);
	~Transform();

	uint64_t id() const;

	void setParent(uint64_t other, bool localize = true);
	void setChild(uint64_t other, bool localize = true);

	void removeParent(bool globalize = true);
	void removeChild(uint32_t i, bool globalize = true);
	void removeChildren(bool globalize = true);

	uint64_t root() const;

	uint64_t parent() const;
	uint64_t child(uint32_t i) const;
	uint32_t children() const;

	void setPosition(const Vec3& position);
	void setRotation(const Quat& rotation);
	void setScale(const Vec3& scale);

	void setWorldPosition(const Vec3& position);
	void setWorldRotation(const Quat& rotation);
	void setWorldScale(const Vec3& scale);

	Vec3 position() const;
	Quat rotation() const;
	Vec3 scale() const;

	Vec3 worldPosition() const;
	Quat worldRotation() const;
	Vec3 worldScale() const;

	Mat4 matrix() const;

	void rotate(const Quat& rotation);
	void translate(const Vec3& translation);
	void scale(const Vec3& scaling);

	void worldRotate(const Quat& rotation);
	void worldTranslate(const Vec3& translation);
	void worldScale(const Vec3& scaling);

	void lookAt(Vec3& position, Vec3 up = WorldVec3::up);
};