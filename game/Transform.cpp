#include "Transform.hpp"

#include <glm\gtc\matrix_transform.hpp>

const glm::vec3 Transform::globalUp(0, 0, 1);
const glm::vec3 Transform::globalDown(0, 0, -1);
const glm::vec3 Transform::globalLeft(-1, 0, 0);
const glm::vec3 Transform::globalRight(1, 0, 0);
const glm::vec3 Transform::globalForward(0, 1, 0);
const glm::vec3 Transform::globalBack(0, -1, 0);

const glm::vec3 Transform::localUp(0, 1, 0);
const glm::vec3 Transform::localDown(0, -1, 0);
const glm::vec3 Transform::localLeft(-1, 0, 0);
const glm::vec3 Transform::localRight(1, 0, 0);
const glm::vec3 Transform::localForward(0, 0, -1);
const glm::vec3 Transform::localBack(0, 0, 1);

glm::mat4 Transform::globalMatrix(const SystemInterface::Engine& engine) const {
	glm::vec3 globalPosition = position;
	glm::quat globalRotation = rotation;
	glm::vec3 globalScale = scale;

	const Transform* parent = engine.getComponent<Transform>(parentId);

	while (parent != nullptr) {
		globalPosition = parent->position + parent->rotation * globalPosition;
		globalRotation = parent->rotation * globalRotation;
		globalScale *= parent->scale;

		parent = engine.getComponent<Transform>(parent->parentId);
	}

	glm::mat4 matrix;

	matrix = glm::translate(matrix, globalPosition);
	matrix *= glm::mat4_cast(globalRotation);
	matrix = glm::scale(matrix, globalScale);

	return matrix;
}

void Transform::localRotate(const glm::quat& rotate) {
	rotation = rotation * rotate;
}

void Transform::localTranslate(const glm::vec3& translation) {
	position = position + rotation * translation;
}

void Transform::localScale(const glm::vec3 & scaling) {
	scale = scale * scaling;
}

void Transform::globalRotate(const glm::quat& rotate) {
	//if (!parent)
		rotation = rotate * rotation;
	//else
	//	_setRotation(glm::inverse(parent->worldRotation()) * (rotation * worldRotation()));
}

void Transform::globalTranslate(const glm::vec3& translation) {
	//if (!parent)
		position = position + translation;
	//else
	//	_setPosition(_position + glm::inverse(parent->worldRotation()) * translation);
}

void Transform::globalScale(const glm::vec3 & scaling) {
	//if (!parent)
		scale = scale * scaling;
	//else
	//	_setScale(scaling / parent->worldScale());
}