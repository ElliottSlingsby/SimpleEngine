#include "Transform.hpp"

#include <glm\gtc\matrix_transform.hpp>

void Transform::_setPosition(const glm::vec3& position) {
	_position = position;
}

void Transform::_setRotation(const glm::quat& rotation) {
	_rotation = rotation;
}

void Transform::_setScale(const glm::vec3 & scale){
	_scale = scale;
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

Transform::Transform(SystemInterface::Engine& engine, uint64_t id) : _engine(engine), _id(id) { }

Transform::~Transform() {
	std::vector<uint64_t> children(_children.begin(), _children.end());

	for (uint64_t child : children)
		_engine.destroyEntity(child);

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

	Transform* otherTransform = _engine.addComponent<Transform>(other, _engine, other);

	if (localize)
		_makeLocal(otherTransform);

	otherTransform->_children.push_back(_id);
	_parent = otherTransform->_id;
}

void Transform::setChild(uint64_t other, bool localize) {
	if (_id == other)
		return;

	Transform* otherTransform = _engine.addComponent<Transform>(other, _engine, other);

	if (localize)
		otherTransform->_makeLocal(this);

	if (std::find(_children.begin(), _children.end(), otherTransform->_id) != _children.end())
		return;

	_children.push_back(otherTransform->_id);
	otherTransform->setParent(_id);
}

void Transform::removeParent(bool globalize) {
	if (!_parent)
		return;
	
	if (globalize)
		_makeGlobal();

	Transform* parent = _engine.getComponent<Transform>(_parent);

	parent->_children.erase(std::find(parent->_children.begin(), parent->_children.end(), _id));
	_parent = 0;
}

void Transform::removeChild(uint32_t i, bool globalize){
	if (i >= _children.size())
		return;

	Transform* child = _engine.getComponent<Transform>(_children[i]);

	if (globalize)
		child->_makeGlobal();

	child->_parent = 0;
	_children.erase(_children.begin() + i);
}

void Transform::removeChildren(bool globalize) {
	if (!_children.size())
		return;

	for (uint64_t child : _children) {
		Transform* transform = _engine.getComponent<Transform>(child);

		if (globalize)
			transform->_makeGlobal();

		transform->_parent = 0;
	}

	_children.clear();
}

uint64_t Transform::root() const {
	uint64_t id = _id;

	Transform* transform = _engine.getComponent<Transform>(id);

	while (transform->_parent) {
		id = transform->_parent;
		transform = _engine.getComponent<Transform>(id);
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
	assert(_children.size() <= UINT32_MAX);
	return static_cast<uint32_t>(_children.size());
}

void Transform::setPosition(const glm::vec3& position) {
	_setPosition(position);
}

void Transform::setRotation(const glm::quat& rotation) {
	_setRotation(rotation);
}

void Transform::setScale(const glm::vec3& scale) {
	_setScale(scale);
}

void Transform::setWorldPosition(const glm::vec3 & position){
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setPosition(position);
	else 
		_setPosition(glm::inverse(parent->worldRotation()) * (position - parent->worldPosition()));
}

void Transform::setWorldRotation(const glm::quat & rotation){
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setRotation(rotation);
	else
		_setRotation(glm::inverse(parent->worldRotation()) * rotation);
}

void Transform::setWorldScale(const glm::vec3 & scale){
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setScale(scale);
	else
		_setScale(scale / parent->worldScale());
}

glm::vec3 Transform::position() const {
	return _position;
}

glm::quat Transform::rotation() const {
	return _rotation;
}

glm::vec3 Transform::scale() const {
	return _scale;
}

glm::vec3 Transform::worldPosition() const {
	glm::vec3 position = _position;
	Transform* parent = _engine.getComponent<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		position = parent->_position + parent->_rotation * position;
		parent = _engine.getComponent<Transform>(parent->_parent);
	}

	return position;
}

glm::quat Transform::worldRotation() const {
	glm::quat rotation = _rotation;
	Transform* parent = _engine.getComponent<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		rotation = parent->_rotation * rotation;
		parent = _engine.getComponent<Transform>(parent->_parent);
	}

	return rotation;
}

glm::vec3 Transform::worldScale() const {
	glm::vec3 scale = _scale;
	Transform* parent = _engine.getComponent<Transform>(_parent);

	while (parent != nullptr && parent->id()) {
		scale *= parent->_scale;
		parent = _engine.getComponent<Transform>(parent->_parent);
	}

	return scale;
}

glm::mat4 Transform::matrix() const {
	glm::mat4 matrix;
	matrix = glm::translate(matrix, worldPosition());
	matrix *= glm::mat4_cast(worldRotation());
	matrix = glm::scale(matrix, worldScale());

	return matrix;
}

void Transform::rotate(const glm::quat& rotation) {
	_setRotation(_rotation * rotation);
}

void Transform::translate(const glm::vec3& translation) {
	_setPosition(_position + _rotation * translation);
}

void Transform::scale(const glm::vec3 & scaling){
	_setScale(_scale * scaling);
}

void Transform::worldRotate(const glm::quat& rotation) {
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setRotation(rotation * _rotation);
	else 
		_setRotation(glm::inverse(parent->worldRotation()) * (rotation * worldRotation()));
}

void Transform::worldTranslate(const glm::vec3& translation) {
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setPosition(_position + translation);
	else
		_setPosition(_position + glm::inverse(parent->worldRotation()) * translation);
}

void Transform::worldScale(const glm::vec3 & scaling){
	Transform* parent = _engine.getComponent<Transform>(_parent);

	if (!parent)
		_setScale(_scale * scaling);
	else
		_setScale(scaling / parent->worldScale());
}

void Transform::lookAt(glm::vec3& position, glm::vec3 up){
	setWorldRotation(glm::inverse(glm::quat_cast(glm::lookAt(worldPosition(), position, up))));
}