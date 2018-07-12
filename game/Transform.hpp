#pragma once

#include "SystemInterface.hpp"

#include <vector>

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

using Number = double;

using Vec2 = glm::tvec2<Number>;
using Vec3 = glm::tvec3<Number>;
using Vec4 = glm::tvec4<Number>;
using Mat4 = glm::tmat4x4<Number>;
using Quat = glm::tquat<Number>;

namespace WorldVec3 {
	const Vec3 up(0, 0, 1);
	const Vec3 down(0, 0, -1);
	const Vec3 left(-1, 0, 0);
	const Vec3 right(1, 0, 0);
	const Vec3 forward(0, 1, 0);
	const Vec3 back(0, -1, 0);
}

namespace LocalVec3 {
	const Vec3 up(0, 1, 0);
	const Vec3 down(0, -1, 0);
	const Vec3 left(-1, 0, 0);
	const Vec3 right(1, 0, 0);
	const Vec3 forward(0, 0, -1);
	const Vec3 back(0, 0, 1);
}

class Transform{
	SystemInterface::Engine& _engine;
	const uint64_t _id;

	Vec3 _position;
	Quat _rotation;
	Vec3 _scale = { 1, 1, 1 };

	uint64_t _parent = 0;
	std::vector<uint64_t> _children;

	void _setPosition(const Vec3& position);
	void _setRotation(const Quat& rotation);
	void _setScale(const Vec3& scale);

	void _makeGlobal();
	void _makeLocal(Transform* transform);

public:
	Transform(SystemInterface::Engine& engine, uint64_t id);
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