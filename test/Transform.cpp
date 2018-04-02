#include "Transform.hpp"

void Transform::getWorldTransform(btTransform& transform) const {
	transform.setOrigin(toBt(worldPosition()));
	transform.setRotation(toBt(worldRotation()));
}

void Transform::setWorldTransform(const btTransform& transform) {
	if (!_parent) {
		_position = toGlm<double>(transform.getOrigin());
		_rotation = toGlm<double>(transform.getRotation());
	}
	else {
		Transform* parent = _engine.entities.get<Transform>(_parent);
		glm::dquat parentRotation = parent->worldRotation();

		_position = (toGlm<double>(transform.getOrigin()) - parent->worldPosition()) * parentRotation;
		_rotation = glm::inverse(parentRotation) * toGlm<double>(transform.getRotation());
	}
}

void Transform::_setPosition(const glm::dvec3& position) {
	glm::dvec3 difference = position - _position;
	_position = position;
	
	if (_engine.entities.has<Collider>(_id)) 
		_engine.entities.get<Collider>(_id)->setWorldPosition(worldPosition());
}

void Transform::_setRotation(const glm::dquat& rotation) {
	_rotation = rotation;
	
	if (_engine.entities.has<Collider>(_id))
		_engine.entities.get<Collider>(_id)->setWorldRotation(worldRotation());
}

Transform::Transform(Engine::EntityManager& entities, uint64_t id) : _engine(*static_cast<Engine*>(entities.enginePtr())), _id(id) { }

Transform::~Transform() {
	if (_engine.entities.has<Collider>(_id))
		_engine.entities.remove<Collider>(_id);

	std::vector<uint64_t> children(_children.begin(), _children.end());

	for (uint64_t child : children)
		_engine.entities.erase(child);

	removeParent();
	removeChildren();
}

uint64_t Transform::id() const{
	return _id;
}

void Transform::setParent(uint64_t other) {
	if (_parent)
		removeParent();

	Transform* otherTransform = _engine.entities.add<Transform>(other);

	otherTransform->_children.push_back(_id);
	_parent = otherTransform->_id;
}

void Transform::setChild(uint64_t other) {
	Transform* otherTransform = _engine.entities.add<Transform>(other);

	if (std::find(_children.begin(), _children.end(), otherTransform->_id) != _children.end())
		return;

	_children.push_back(otherTransform->_id);
	otherTransform->setParent(_id);
}

void Transform::removeParent() {
	if (!_parent)
		return;
	
	Transform* parent = _engine.entities.get<Transform>(_parent);

	parent->_children.erase(std::find(parent->_children.begin(), parent->_children.end(), _id));

	_parent = 0;
}

void Transform::removeChild(uint32_t i){
	if (i >= _children.size())
		return;

	Transform* child = _engine.entities.get<Transform>(_children[i]);
	child->_parent = 0;

	_children.erase(_children.begin() + i);
}

void Transform::removeChildren() {
	if (!_children.size())
		return;

	for (uint64_t child : _children)
		_engine.entities.get<Transform>(child)->_parent = 0;

	_children.clear();
}

uint64_t Transform::root() const {
	uint64_t id = _id;

	while (uint64_t parent = _engine.entities.get<Transform>(id)->_parent)
		id = parent;

	return id;
}

uint64_t Transform::parent() {
	return _parent;
}

uint64_t Transform::child(uint32_t i) {
	if (i >= _children.size())
		return 0;

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

glm::dvec3 Transform::relativePosition(uint64_t to) const {
	glm::dvec3 position = _position;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr) {
		position = parent->_position + parent->_rotation * position;
		parent = _engine.entities.get<Transform>(parent->_parent);

		if (parent && parent->id() == to)
			break;
	}

	return position;
}

glm::dquat Transform::relativeRotation(uint64_t to) const {
	glm::dquat rotation = _rotation;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr) {
		rotation = parent->_rotation * rotation;
		parent = _engine.entities.get<Transform>(parent->_parent);

		if (parent && parent->id() == to)
			break;
	}

	return rotation;
}

glm::dvec3 Transform::relativeScale(uint64_t to) const {
	glm::dvec3 scale = _scale;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr) {
		scale *= parent->_scale;
		parent = _engine.entities.get<Transform>(parent->_parent);

		if (parent && parent->id() == to)
			break;
	}

	return scale;
}

glm::dvec3 Transform::worldPosition() const {
	if (!hasFlags(_inheritanceFlags, Position))
		return _position;

	return relativePosition(0);
}

glm::dquat Transform::worldRotation() const {
	if (!hasFlags(_inheritanceFlags, Rotation))
		return _rotation;

	return relativeRotation(0);
}

glm::dvec3 Transform::worldScale() const {
	if (!hasFlags(_inheritanceFlags, Scale))
		return _scale;

	return relativeScale(0);
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