#include "Transform.hpp"

#include "Physics.hpp"

void Transform::getWorldTransform(btTransform& transform) const {
	if (_engine.entities.has<Collider>(_id) && !_parent)
		transform.setOrigin(toBt(worldPosition() + _rotation * _engine.entities.get<Collider>(_id)->centerOfMass()));
	else
		transform.setOrigin(toBt(worldPosition()));

	transform.setRotation(toBt(worldRotation()));
}

void Transform::setWorldTransform(const btTransform& transform) {
	if (!_parent) {
		Collider* collider = _engine.entities.get<Collider>(_id);

		if (collider)
			_position = toGlm<double>(transform.getOrigin()) - _rotation * collider->centerOfMass();
		else
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
	_position = position;
	
	if (_engine.entities.has<Collider>(_id)) 
		_engine.system<Physics>().updateWorldTransform(_id);
}

void Transform::_setRotation(const glm::dquat& rotation) {
	_rotation = rotation;
	
	if (_engine.entities.has<Collider>(_id))
		_engine.system<Physics>().updateWorldTransform(_id);
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
	if (_id == other)
		return;

	if (_parent)
		removeParent();

	Transform* otherTransform = _engine.entities.add<Transform>(other);

	otherTransform->_children.push_back(_id);
	_parent = otherTransform->_id;

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::setChild(uint64_t other) {
	if (_id == other)
		return;

	Transform* otherTransform = _engine.entities.add<Transform>(other);

	if (std::find(_children.begin(), _children.end(), otherTransform->_id) != _children.end())
		return;

	_children.push_back(otherTransform->_id);
	otherTransform->setParent(_id);

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeParent() {
	if (!_parent)
		return;
	
	Transform* parent = _engine.entities.get<Transform>(_parent);

	parent->_children.erase(std::find(parent->_children.begin(), parent->_children.end(), _id));

	_engine.system<Physics>().updateCompoundShape(_parent); // rebuild detached parent

	_parent = 0;

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeChild(uint32_t i){
	if (i >= _children.size())
		return;

	Transform* child = _engine.entities.get<Transform>(_children[i]);
	child->_parent = 0;

	_engine.system<Physics>().updateCompoundShape(_children[i]); // rebuild detached child

	_children.erase(_children.begin() + i);

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeChildren() {
	if (!_children.size())
		return;

	for (uint64_t child : _children) {
		_engine.entities.get<Transform>(child)->_parent = 0;
		_engine.system<Physics>().updateCompoundShape(child); // rebuild detached child
	}

	_children.clear();

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

uint64_t Transform::root() const {
	uint64_t id = _id;

	Transform* transform = _engine.entities.get<Transform>(id);

	while (transform->_parent) {
		id = transform->_parent;
		transform = _engine.entities.get<Transform>(id);
	}

	return id;
}

uint64_t Transform::parent() const {
	return _parent;
}

uint64_t Transform::child(uint32_t i) const {
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
	if (_id == to)
		return glm::dvec3();

	glm::dvec3 position = _position;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id() != to) {
		position = parent->_position + parent->_rotation * position;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return position;
}

glm::dquat Transform::relativeRotation(uint64_t to) const {
	if (_id == to)
		return glm::dquat();

	glm::dquat rotation = _rotation;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id() != to) {
		rotation = parent->_rotation * rotation;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return rotation;
}

glm::dvec3 Transform::relativeScale(uint64_t to) const {
	if (_id == to)
		return glm::dvec3(1, 1, 1);

	glm::dvec3 scale = _scale;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id() != to) {
		scale *= parent->_scale;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return scale;
}

glm::dvec3 Transform::worldPosition() const {
	if (!hasFlags(_inheritanceFlags, Position))
		return position();
	
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