#include "Transform.hpp"

void Transform::_setPosition(const glm::dvec3& position) {
	_position = position;

	if (_collider) {
		btTransform transform = _collider->_rigidBody->getWorldTransform();

		transform.setOrigin(toBt(position));
		_collider->_rigidBody->setWorldTransform(transform);
	}
}

void Transform::_setRotation(const glm::dquat& rotation) {
	_rotation = rotation;

	if (_collider) {
		btTransform transform = _collider->_rigidBody->getWorldTransform();

		transform.setRotation(toBt(rotation));
		_collider->_rigidBody->setWorldTransform(transform);
	}
}

Transform::Transform(Engine::EntityManager& entities, uint64_t id) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id) {
	if (_engine.entities.has<Collider>(_id))
		_collider = _engine.entities.get<Collider>(_id);
}

Transform::~Transform() {
	if (_collider)
		_engine.entities.remove<Collider>(_id);

	removeParent();
	removeChildren();
}

uint64_t Transform::id() const{
	return _id;
}

void Transform::setParent(Transform* other) {
	if (_parent)
		removeParent();

	other->_children.push_back(this);
	_parent = other;
}

void Transform::setChild(Transform* other) {
	if (std::find(_children.begin(), _children.end(), other) != _children.end())
		return;

	_children.push_back(other);
	other->setParent(this);
}

void Transform::removeParent() {
	if (_parent)
		_parent->_children.erase(std::find(_parent->_children.begin(), _parent->_children.end(), this));

	_parent = nullptr;
}

void Transform::removeChildren() {
	for (Transform* child : _children)
		child->_parent = nullptr;

	_children.clear();
}

void Transform::setPosition(const glm::dvec3& position) {
	_setPosition(position);
}

void Transform::setRotation(const glm::dquat& rotation) {
	_setRotation(rotation);
}

void Transform::setScale(const glm::dvec3& scale) {
	_scale = scale;
}

glm::dvec3 Transform::position() const {
	return _position;
}

glm::dquat Transform::rotation() const {
	return _rotation;
}

glm::dvec3 Transform::scale() const {
	return _scale;
}

glm::dvec3 Transform::worldPosition() const {
	glm::dvec3 position = _position;
	Transform* rParent = _parent;

	while (rParent != nullptr) {
		position = rParent->_position + rParent->_rotation * position;
		rParent = rParent->_parent;
	}

	return position;
}

glm::dquat Transform::worldRotation() const {
	glm::dquat rotation = _rotation;
	Transform* rParent = _parent;

	while (rParent != nullptr) {
		rotation = rParent->_rotation * rotation;
		rParent = rParent->_parent;
	}

	return rotation;
}

glm::dvec3 Transform::worldScale() const {
	glm::dvec3 scale = _scale;
	Transform* rParent = _parent;

	while (rParent != nullptr) {
		scale *= rParent->_scale;
		rParent = rParent->_parent;
	}

	return scale;
}

void Transform::rotate(const glm::dquat& rotation) {
	_setRotation(_rotation * rotation);
}

void Transform::translate(const glm::dvec3& translation) {
	_setPosition(_position + _rotation * translation);
}

void Transform::globalRotate(const glm::dquat& rotation) {
	_setRotation(rotation * _rotation);
}

void Transform::globalTranslate(const glm::dvec3& translation) {
	_setPosition(_position + translation);
}

void Transform::getWorldTransform(btTransform& transform) const {
	transform.setOrigin(toBt(_position));
	transform.setRotation(toBt(_rotation));
}

void Transform::setWorldTransform(const btTransform& transform) {
	_position = toGlm<double>(transform.getOrigin());
	_rotation = toGlm<double>(transform.getRotation());
}