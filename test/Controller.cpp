#include "Controller.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"
#include "Collider.hpp"

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Update, &Controller::update);
	_engine.events.subscribe(this, Events::Cursor, &Controller::cursor);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
}

void Controller::update(double dt) {
	if (!_possessed || !_engine.entities.has<Transform, Collider>(_possessed))
		return;

	Collider& collider = *_engine.entities.get<Collider>(_possessed);

	collider.rigidBody->setAngularVelocity(btVector3(0, 0, 0));
	collider.rigidBody->setLinearVelocity(btVector3(0, 0, 0));

	Transform& transform = *_engine.entities.get<Transform>(_possessed);

	glm::vec3 angles = glm::eulerAngles(transform.rotation());
	glm::quat rotation({ angles.x, 0.f, angles.z });

	glm::vec3 position = transform.position();

	if (_locked) {
		rotation = glm::quat({ 0.f, 0.f, -_dCursor.x / 256.f }) * rotation;
		rotation *= glm::quat({ -_dCursor.y / 256.f, 0.f, 0.f });

		_dCursor = { 0.0, 0.0 };

		float moveSpeed;

		if (_boost)
			moveSpeed = 2.f;
		else
			moveSpeed = 1.f;

		if (_forward)
			position += rotation * (LocalVec3::forward * moveSpeed);
		if (_back)
			position += rotation * (LocalVec3::back * moveSpeed);
		if (_left)
			position += rotation * (LocalVec3::left * moveSpeed);
		if (_right)
			position += rotation * (LocalVec3::right * moveSpeed);
		if (_up)
			position += GlobalVec3::up * moveSpeed;
		if (_down)
			position += GlobalVec3::down * moveSpeed;
	}

	btTransform newTransform;
	newTransform.setOrigin(btVector3(position.x, position.y, position.z));
	newTransform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));

	collider.rigidBody->activate();
	collider.rigidBody->setWorldTransform(newTransform);
}

void Controller::cursor(double x, double y){
	glm::dvec2 newCursor = { x, y };

	if (_cursor == glm::dvec2(0.0, 0.0))
		_cursor = newCursor;

	_dCursor = newCursor - _cursor;
	_cursor = newCursor;
}

void Controller::keypress(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		_locked = !_locked;
		_engine.system<Renderer>().lockCursor(_locked);
		return;
	}

	if (!_possessed)
		return;

	bool value;

	if (action == GLFW_PRESS)
		value = true;
	else if (action == GLFW_RELEASE)
		value = false;
	else
		return;

	if (key == GLFW_KEY_W)
		_forward = value;
	if (key == GLFW_KEY_S)
		_back = value;
	if (key == GLFW_KEY_A)
		_left = value;
	if (key == GLFW_KEY_D)
		_right = value;
	if (key == GLFW_KEY_SPACE)
		_up = value;
	if (key == GLFW_KEY_LEFT_CONTROL)
		_down = value;
	if (key == GLFW_KEY_LEFT_SHIFT)
		_boost = value;
}

void Controller::setPossessed(uint64_t id) {
	if ((id && !_engine.entities.valid(id)) || (id && !_engine.entities.has<Transform>(id)))
		return;

	if (_possessed)
		_engine.entities.dereference(id);

	if (id)
		_engine.entities.reference(id);
	
	_engine.entities.add<Transform>(id);
	_possessed = id;
}
