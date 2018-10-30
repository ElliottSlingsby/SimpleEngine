#pragma once

#include "SystemInterface.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

namespace WorldVec3 {
	const glm::vec3 up(0, 0, 1);
	const glm::vec3 down(0, 0, -1);
	const glm::vec3 left(-1, 0, 0);
	const glm::vec3 right(1, 0, 0);
	const glm::vec3 forward(0, 1, 0);
	const glm::vec3 back(0, -1, 0);
}

namespace LocalVec3 {
	const glm::vec3 up(0, 1, 0);
	const glm::vec3 down(0, -1, 0);
	const glm::vec3 left(-1, 0, 0);
	const glm::vec3 right(1, 0, 0);
	const glm::vec3 forward(0, 0, -1);
	const glm::vec3 back(0, 0, 1);
}

struct Transform{
	static const glm::vec3 localUp;
	static const glm::vec3 localDown;
	static const glm::vec3 localLeft;
	static const glm::vec3 localRight;
	static const glm::vec3 localForward;
	static const glm::vec3 localBack;

	static const glm::vec3 globalUp;
	static const glm::vec3 globalDown;
	static const glm::vec3 globalLeft;
	static const glm::vec3 globalRight;
	static const glm::vec3 globalForward;
	static const glm::vec3 globalBack;

	uint64_t parentId = 0;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale = { 1, 1, 1 };

	glm::mat4 globalMatrix(const SystemInterface::Engine& engine) const;

	void localRotate(const glm::quat& rotation);
	void localTranslate(const glm::vec3& translation);
	void localScale(const glm::vec3 & scaling);

	void globalRotate(const glm::quat& rotation);
	void globalTranslate(const glm::vec3& translation);
	void globalScale(const glm::vec3 & scaling);
};