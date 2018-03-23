#include "Controller.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Update, &Controller::update);
	_engine.events.subscribe(this, Events::Cursor, &Controller::cursor);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
}

void Controller::update(double dt) {
	if (!_locked)
		return;

	Transform& transform = *_engine.entities.get<Transform>(_possessed);

	transform.rotation = glm::quat({ 0.f, 0.f, glm::radians((20.0 *-_dCursor.x) * dt) }) * transform.rotation;
	transform.rotation *= glm::quat({ glm::radians((20.0 *-_dCursor.y) * dt), 0.f, 0.f });

	glm::vec3 angles = glm::eulerAngles(transform.rotation);
	transform.rotation = glm::quat({ glm::clamp(glm::abs(angles.x), 0.f, glm::pi<float>()), angles.y, angles.z });

	_dCursor = { 0.0, 0.0 };

	glm::dquat rotation = static_cast<glm::dquat>(transform.rotation);
	
	double moveSpeed;
	
	if (_boost)
		moveSpeed = 400.0 * dt;
	else
		moveSpeed = 100.0 * dt;

	if (_forward)
		transform.position += rotation * (LocalDVec3::forward * moveSpeed);
	if (_back)
		transform.position += rotation * (LocalDVec3::back * moveSpeed);
	if (_left)
		transform.position += rotation * (LocalDVec3::left * moveSpeed);
	if (_right)
		transform.position += rotation * (LocalDVec3::right * moveSpeed);
	if (_up)
		transform.position += GlobalDVec3::up * moveSpeed;
	if (_down)
		transform.position += GlobalDVec3::down * moveSpeed;
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

	switch (key) {
	case GLFW_KEY_W:
		_forward = value;
		return;
	case GLFW_KEY_S:
		_back = value;
		return;
	case GLFW_KEY_A:
		_left = value;
		return;
	case GLFW_KEY_D:
		_right = value;
		return;
	case GLFW_KEY_SPACE:
		_up = value;
		return;
	case GLFW_KEY_LEFT_CONTROL:
		_down = value;
		return;
	case GLFW_KEY_LEFT_SHIFT:
		_boost = value;
		return;
	};
}

void Controller::setPossessed(uint64_t id) {
	if ((id && !_engine.entities.valid(id)) || (id && !_engine.entities.has<Transform>(id)))
		return;

	if (_possessed)
		_engine.entities.dereference(id);

	if (id)
		_engine.entities.reference(id);
	
	_possessed = id;
}
