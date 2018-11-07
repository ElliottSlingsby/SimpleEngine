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

Transform::Transform(SystemInterface::Engine& engine, uint64_t id) : _engine(engine), _id(id) { }

Transform::~Transform() {
	removeParent();
	removeChildren();
}

void Transform::addChild(uint64_t childId) {
	Transform* newChild = _engine.getComponent<Transform>(childId);

	// if no transform, or already a child of this
	if (!newChild || newChild->_parent == _id)
		return;

	// if child of something else
	if (newChild->_parent)
		newChild->removeParent();

	// set parent
	newChild->_parent = _id;

	// if first child
	if (!_firstChild) {
		newChild->_leftSibling = childId;
		newChild->_rightSibling = childId;
		_firstChild = childId;

		return;
	}

	Transform* firstChild = _engine.getComponent<Transform>(_firstChild);
	Transform* secondChild;

	// if only one sibling
	if (firstChild->_rightSibling == _firstChild)
		secondChild = firstChild;
	else
		secondChild = _engine.getComponent<Transform>(firstChild->_rightSibling);

	// add into linked list after first child   O O O O --> O X O O O
	newChild->_leftSibling = _firstChild;
	newChild->_rightSibling = firstChild->_rightSibling;

	firstChild->_rightSibling = childId;
	secondChild->_leftSibling = childId;
}

void Transform::removeParent() {
	if (!_parent)
		return;

	Transform* parent = _engine.getComponent<Transform>(_parent);

	// if only child
	if (_rightSibling == _id) {
		parent->_firstChild = 0;
		_rightSibling = _id;
		_leftSibling = _id;
	}
	else {
		// set parent
		if (parent->_firstChild == _id)
			parent->_firstChild = _rightSibling;

		Transform* leftSibling = _engine.getComponent<Transform>(_leftSibling);
		Transform* rightSibling;

		// if only one sibling
		if (_leftSibling == _rightSibling)
			rightSibling = leftSibling;
		else
			rightSibling = _engine.getComponent<Transform>(_rightSibling);

		// remove from linked list
		leftSibling->_rightSibling = _rightSibling;
		rightSibling->_leftSibling = _leftSibling;
	}

	_parent = 0;
	_rightSibling = _id;
	_leftSibling = _id;
}

void Transform::removeChildren() {
	if (!_firstChild)
		return;

	uint64_t i = _rightSibling;

	while (i != _id) {
		Transform* sibling = _engine.getComponent<Transform>(_rightSibling);

		i = sibling->_rightSibling;

		sibling->_parent = 0;
		sibling->_leftSibling = sibling->_id;
		sibling->_rightSibling = sibling->_id;
	}

	_firstChild = 0;
}

void Transform::_getChildren(uint64_t id, uint32_t count, std::vector<uint64_t>* ids) const {
	Transform* sibling = _engine.getComponent<Transform>(id);

	if (sibling->_rightSibling == _firstChild)
		ids->resize(count + 1);
	else
		_getChildren(sibling->_rightSibling, count + 1, ids);

	ids->at(count) = sibling->_id;
}

bool Transform::hasChildren() const {
	return _firstChild;
}

void Transform::getChildren(std::vector<uint64_t>* ids) const {
	if (!_firstChild)
		return;

	_getChildren(_firstChild, 0, ids);
}

glm::mat4 Transform::globalMatrix() const {
	glm::vec3 globalPosition = position;
	glm::quat globalRotation = rotation;
	glm::vec3 globalScale = scale;

	const Transform* parent = _engine.getComponent<Transform>(_parent);

	while (parent != nullptr) {
		globalPosition = (parent->position + parent->rotation * globalPosition);// *parent->scale;
		globalRotation = parent->rotation * globalRotation;
		globalScale *= parent->scale;

		parent = _engine.getComponent<Transform>(parent->_parent);
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