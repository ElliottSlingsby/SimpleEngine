#include "Controller.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Update, &Controller::update);
	_engine.events.subscribe(this, Events::Cursor, &Controller::cursor);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
}

void Controller::update(double dt) {
	Transform& transform = *_engine.entities.get<Transform>(_possessed);

	if (_locked) {
		transform.rotation = glm::quat({ 0.f, 0.f, glm::radians((15.0 *-_dCursor.x) * dt) }) * transform.rotation;
		transform.rotation *= glm::quat({ glm::radians((15.0 *-_dCursor.y) * dt), 0.f, 0.f });

		_dCursor = { 0.0, 0.0 };
	}

	glm::dquat rotation = static_cast<glm::dquat>(transform.rotation);
	double moveSpeed = 100.0 * dt;

	if (_forward)
		transform.position += rotation * (downVecD * moveSpeed);
	if (_back)
		transform.position += rotation * (upVecD * moveSpeed);
	if (_left)
		transform.position += rotation * (leftVecD * moveSpeed);
	if (_right)
		transform.position += rotation * (rightVecD * moveSpeed);
	if (_up)
		transform.position += upVecD * moveSpeed;
	if (_down)
		transform.position += downVecD * moveSpeed;
}

void Controller::cursor(double x, double y){
	glm::dvec2 newCursor = { x, y };

	if (_cursor == glm::dvec2(0.0, 0.0))
		_cursor = newCursor;

	_dCursor = newCursor - _cursor;
	_cursor = newCursor;

	//std::cout << _dCursor.x << " - " << _dCursor.y << '\n';
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
		break;
	case GLFW_KEY_LEFT_CONTROL:
		_down = value;
		break;
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
