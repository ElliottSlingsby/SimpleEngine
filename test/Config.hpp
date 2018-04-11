#pragma once

#include <SimpleEngine.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

#define MAX_SYSTEMS 8
#define MAX_COMPONENTS 16
#define MAX_EVENTS 32
#define MAX_LISTENERS 8

using Engine = SimpleEngine<MAX_SYSTEMS, MAX_COMPONENTS, MAX_EVENTS, MAX_LISTENERS>;

enum Events {
	Load,
	Input,
	Update,
	Reset,

	Keypress,
	Mousemove,
	Mousepress,
	Windowsize,
};

const size_t chunkSize = 1024 * 1024 * 128; // 128 mb

#define DATA_FOLDER "data"

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