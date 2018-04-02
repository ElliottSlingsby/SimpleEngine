#include "Transform.hpp"

void Transform::getWorldTransform(btTransform& transform) const {
	transform.setOrigin(toBt(worldPosition()));
	transform.setRotation(toBt(worldRotation()));
}

void Transform::setWorldTransform(const btTransform& transform) {
	_position = glm::dvec3();
	glm::dvec3 parentPosition = worldPosition();

	_rotation = glm::dquat();
	glm::dquat parentRotation = worldRotation();
	
	_position = (toGlm<double>(transform.getOrigin()) - parentPosition) * parentRotation;
	_rotation = glm::inverse(parentRotation) * toGlm<double>(transform.getRotation());
}

void Transform::_setPosition(const glm::dvec3& position) {
	glm::dvec3 difference = position - _position;
	_position = position;
	
	if (_collider) 
		_collider->_setWorldPosition();
}

void Transform::_setRotation(const glm::dquat& rotation) {
	_rotation = rotation;
	
	if (_collider)
		_collider->_setWorldRotation();
}

void Transform::_setCollider(Collider* collider) {
	_collider = collider;
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

	if (_collider)
		_collider->_rebuildCompoundShape();
}

void Transform::setChild(Transform* other) {
	if (std::find(_children.begin(), _children.end(), other) != _children.end())
		return;

	_children.push_back(other);
	other->setParent(this);

	if (_collider)
		_collider->_rebuildCompoundShape();
}

void Transform::removeParent() {
	if (!_parent)
		return;
	
	_parent->_children.erase(std::find(_parent->_children.begin(), _parent->_children.end(), this));

	if (_parent->_collider)
		_parent->_collider->_rebuildCompoundShape();

	_parent = nullptr;

	if (_collider) 
		_collider->_rebuildCompoundShape();
}

void Transform::removeChild(uint32_t i){
	if (i >= _children.size())
		return;

	Transform* child = _children[i];
	child->_parent = nullptr;

	if (child->_collider)
		child->_collider->_rebuildCompoundShape();

	_children.erase(_children.begin() + i);

	if (_collider) 
		_collider->_rebuildCompoundShape();
}

void Transform::removeChildren() {
	if (!_children.size())
		return;

	for (Transform* child : _children) {
		child->_parent = nullptr;
		
		if (child->_collider)
			child->_collider->_rebuildCompoundShape();
	}

	_children.clear();

	if (_collider)
		_collider->_rebuildCompoundShape();
}

Transform* Transform::parent() {
	return _parent;
}

Transform* Transform::child(uint32_t i) {
	if (i >= _children.size())
		return nullptr;

	return _children[i];
}

uint32_t Transform::children() const {
	return static_cast<uint32_t>(_children.size());
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
	if (!hasFlags(_inheritanceFlags, Position))
		return _position;

	glm::dvec3 position = _position;
	Transform* rParent = _parent;

	while (rParent != nullptr) {
		position = rParent->_position + rParent->_rotation * position;
		rParent = rParent->_parent;
	}

	return position;
}

glm::dquat Transform::worldRotation() const {
	if (!hasFlags(_inheritanceFlags, Rotation))
		return _rotation;

	glm::dquat rotation = _rotation;
	Transform* rParent = _parent;

	while (rParent != nullptr) {
		rotation = rParent->_rotation * rotation;
		rParent = rParent->_parent;
	}

	return rotation;
}

glm::dvec3 Transform::worldScale() const {
	if (!hasFlags(_inheritanceFlags, Scale))
		return _scale;

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

void Transform::inheritPosition(bool value) {
	if (value)
		_inheritanceFlags |= Position;
	else
		_inheritanceFlags &= ~Position;
}

void Transform::inheritRotation(bool value) {
	if (value)
		_inheritanceFlags |= Rotation;
	else
		_inheritanceFlags &= ~Rotation;
}

void Transform::inheritScale(bool value) {
	if (value)
		_inheritanceFlags |= Scale;
	else
		_inheritanceFlags &= ~Scale;
}