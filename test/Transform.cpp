#include "Transform.hpp"

#include "Physics.hpp"
#include <glm\gtc\matrix_transform.hpp>

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
			_position = fromBt<double>(transform.getOrigin()) - _rotation * collider->centerOfMass();
		else
			_position = fromBt<double>(transform.getOrigin());

		_rotation = fromBt<double>(transform.getRotation());
	}
	else {
		Transform* parent = _engine.entities.get<Transform>(_parent);
		glm::dquat parentRotation = parent->worldRotation();

		_position = (fromBt<double>(transform.getOrigin()) - parent->worldPosition()) * parentRotation;
		_rotation = glm::inverse(parentRotation) * fromBt<double>(transform.getRotation());
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

void Transform::_setScale(const glm::dvec3 & scale){
	_scale = scale;

	if (_engine.entities.has<Collider>(_id))
		_engine.system<Physics>().updateWorldTransform(_id);
}

void Transform::_makeGlobal(){
	_position = worldPosition();
	_rotation = worldRotation();
	_scale = worldScale();
}

void Transform::_makeLocal(Transform* transform){
	_position = glm::inverse(transform->worldRotation()) * (worldPosition() - transform->worldPosition());
	_rotation = glm::inverse(transform->worldRotation()) * worldRotation();
	_scale = worldScale() / transform->worldScale();
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

void Transform::setParent(uint64_t other, bool localize) {
	if (_id == other)
		return;

	if (_parent)
		removeParent();

	Transform* otherTransform = _engine.entities.add<Transform>(other);

	if (localize)
		_makeLocal(otherTransform);

	otherTransform->_children.push_back(_id);
	_parent = otherTransform->_id;

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::setChild(uint64_t other, bool localize) {
	if (_id == other)
		return;

	Transform* otherTransform = _engine.entities.add<Transform>(other);

	if (localize)
		otherTransform->_makeLocal(this);

	if (std::find(_children.begin(), _children.end(), otherTransform->_id) != _children.end())
		return;

	_children.push_back(otherTransform->_id);
	otherTransform->setParent(_id);

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeParent(bool globalize) {
	if (!_parent)
		return;
	
	if (globalize)
		_makeGlobal();

	Transform* parent = _engine.entities.get<Transform>(_parent);

	parent->_children.erase(std::find(parent->_children.begin(), parent->_children.end(), _id));

	_engine.system<Physics>().updateCompoundShape(_parent); // rebuild detached parent

	_parent = 0;

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeChild(uint32_t i, bool globalize){
	if (i >= _children.size())
		return;

	Transform* child = _engine.entities.get<Transform>(_children[i]);

	if (globalize)
		child->_makeGlobal();

	child->_parent = 0;

	_engine.system<Physics>().updateCompoundShape(_children[i]); // rebuild detached child

	_children.erase(_children.begin() + i);

	_engine.system<Physics>().updateCompoundShape(_id); // rebuild self
}

void Transform::removeChildren(bool globalize) {
	if (!_children.size())
		return;

	for (uint64_t child : _children) {
		Transform* transform = _engine.entities.get<Transform>(child);

		if (globalize)
			transform->_makeGlobal();

		transform->_parent = 0;

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

void Transform::setWorldPosition(const glm::dvec3 & position){
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setPosition(position);
	else 
		_setPosition(glm::inverse(parent->worldRotation()) * (position - parent->worldPosition()));
}

void Transform::setWorldRotation(const glm::dquat & rotation){
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setRotation(rotation);
	else
		_setRotation(glm::inverse(parent->worldRotation()) * rotation);
}

void Transform::setWorldScale(const glm::dvec3 & scale){
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setScale(scale);
	else
		_setScale(scale / parent->worldScale());
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
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		position = parent->_position + parent->_rotation * position;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return position;
}

glm::dquat Transform::worldRotation() const {
	glm::dquat rotation = _rotation;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		rotation = parent->_rotation * rotation;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return rotation;
}

glm::dvec3 Transform::worldScale() const {
	glm::dvec3 scale = _scale;
	Transform* parent = _engine.entities.get<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		scale *= parent->_scale;
		parent = _engine.entities.get<Transform>(parent->_parent);
	}

	return scale;
}

glm::dmat4 Transform::matrix() const {
	glm::dmat4 matrix;
	matrix = glm::translate(matrix, worldPosition());
	matrix *= glm::mat4_cast(worldRotation());
	matrix = glm::scale(matrix, worldScale());

	return matrix;
}

void Transform::rotate(const glm::dquat& rotation) {
	_setRotation(_rotation * rotation);
}

void Transform::translate(const glm::dvec3& translation) {
	_setPosition(_position + _rotation * translation);
}

void Transform::scale(const glm::dvec3 & scaling){
	_setScale(_scale * scaling);
}

void Transform::worldRotate(const glm::dquat& rotation) {
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setRotation(rotation * _rotation);
	else 
		_setRotation(glm::inverse(parent->worldRotation()) * (rotation * worldRotation()));
}

void Transform::worldTranslate(const glm::dvec3& translation) {
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setPosition(_position + translation);
	else
		_setPosition(_position + glm::inverse(parent->worldRotation()) * translation);
}

void Transform::worldScale(const glm::dvec3 & scaling){
	Transform* parent = _engine.entities.get<Transform>(_parent);

	if (!parent)
		_setScale(_scale * scaling);
	else
		_setScale(scaling / parent->worldScale());
}

void Transform::lookAt(glm::dvec3& position, glm::dvec3 up){
	setWorldRotation(glm::inverse(glm::quat_cast(glm::lookAt(worldPosition(), position, up))));
}