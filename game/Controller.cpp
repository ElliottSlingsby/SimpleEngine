#include "Controller.hpp"

#include "Transform.hpp"
#include "Window.hpp"

Controller::Controller(Engine& engine) : _engine(engine), _possessed(engine), _cursor(engine){
	SYSFUNC_ENABLE(SystemInterface, update, 0);

	SYSFUNC_ENABLE(SystemInterface, cursorPosition, 0);
	SYSFUNC_ENABLE(SystemInterface, keyInput, 0);
	SYSFUNC_ENABLE(SystemInterface, cursorEnter, 0);
	SYSFUNC_ENABLE(SystemInterface, mousePress, 0);

	assert(_engine.hasSystem<Window>());
}

void Controller::update(double dt) {
	if (!_possessed.valid())
		return;

	Transform* possessedTransform = _possessed.get<Transform>();

	if (!possessedTransform)
		return;

	if (_locked) {
		possessedTransform->worldRotate(glm::quat({ 0.0, 0.0, -_dMousePos.x * dt }));
		possessedTransform->rotate(glm::quat({ -_dMousePos.y * dt, 0.0, 0.0 }));

		_dMousePos = { 0.0, 0.0 };

		double moveSpeed;

		if (_boost)
			moveSpeed = 500.0 * dt;
		else
			moveSpeed = 100.0 * dt;

		if (_forward)
			possessedTransform->translate(LocalVec3::forward * (float)moveSpeed);
		if (_back)
			possessedTransform->translate(LocalVec3::back * (float)moveSpeed);
		if (_left)
			possessedTransform->translate(LocalVec3::left * (float)moveSpeed);
		if (_right)
			possessedTransform->translate(LocalVec3::right * (float)moveSpeed);
		if (_up)
			possessedTransform->worldTranslate(WorldVec3::up * (float)moveSpeed);
		if (_down)
			possessedTransform->worldTranslate(WorldVec3::down * (float)moveSpeed);
	}
}

void Controller::cursorPosition(glm::dvec2 position){
	glm::vec2 newMousePos = position;

	if (_mousePos == glm::vec2(0.0, 0.0))
		_mousePos = newMousePos;

	_dMousePos = newMousePos - _mousePos;
	_mousePos = newMousePos;
}

void Controller::keyInput(uint32_t key, Action action, Modifier mods){
	if (action == Action::Release && key == Escape) {
		if (_locked) {
			_engine.system<Window>().lockCursor(false);
			_locked = false;
		}
		else if (!_locked) {
			_engine.quit();
		}

		return;
	}

	if (!_possessed.valid())
		return;

	bool value;

	if (action == Action::Press)
		value = true;
	else if (action == Action::Release)
		value = false;
	else
		return;

	switch (key) {
	case 'W':
		_forward = value;
		break;
	case 'S':
		_back = value;
		break;
	case 'A':
		_left = value;
		break;
	case 'D':
		_right = value;
		break;
	case ' ':
		_up = value;
		break;
	case Key::LeftControl:
		_down = value;
		break;
	case Key::RightControl:
		_boost = value;
		break;
	}
}

void Controller::cursorEnter(bool enterered) {
	_cursorInside = enterered;
}

void Controller::mousePress(uint32_t button, Action action, Modifier mods) {
	if (action == Action::Release && !_locked && _cursorInside) {
		_engine.system<Window>().lockCursor(true);
		_locked = true;
	}
}

void Controller::setPossessed(uint64_t id) {
	_possessed.set(id);
}