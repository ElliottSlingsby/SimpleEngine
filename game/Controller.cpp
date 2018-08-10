#include "Controller.hpp"

#include "Transform.hpp"
#include "Window.hpp"

Controller::Controller(Engine& engine) : _engine(engine), _possessed(engine) {
	SYSFUNC_ENABLE(SystemInterface, update, 0);

	SYSFUNC_ENABLE(SystemInterface, cursorPosition, 0);
	SYSFUNC_ENABLE(SystemInterface, keyInput, 0);
	SYSFUNC_ENABLE(SystemInterface, cursorEnter, 0);
	SYSFUNC_ENABLE(SystemInterface, mousePress, 0);
	//SYSFUNC_ENABLE(SystemInterface, windowOpen, 0);
	SYSFUNC_ENABLE(SystemInterface, scrollWheel, 0);

	assert(_engine.hasSystem<Window>());
}

void Controller::update(double dt) {
	Transform* possessedTransform = nullptr;
	
	if (_possessed.valid())
		possessedTransform = _possessed.get<Transform>();

	if (possessedTransform && _locked) {
		possessedTransform->worldRotate(glm::quat({ 0.0, 0.0, -_dMousePos.x * dt }));
		possessedTransform->rotate(glm::quat({ -_dMousePos.y * dt, 0.0, 0.0 }));

		double moveSpeed;

		if (_boost)
			moveSpeed = 500.0 * dt;
		else
			moveSpeed = 100.0 * dt;

		possessedTransform->translate(glm::vec3(0.f, 0.f, -_flash * 100.f));

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

	_flash = 0;
	_dMousePos = { 0.0, 0.0 };
}

void Controller::cursorPosition(glm::dvec2 position){
	/*
	glm::vec2 newMousePos = position;

	if (_mousePos == glm::vec2(0.0, 0.0))
		_mousePos = newMousePos;

	_dMousePos = newMousePos - _mousePos;
	_mousePos = newMousePos;
	*/

	_dMousePos = position;
}

void Controller::keyInput(uint32_t key, Action action, uint8_t mods){
	if (action == Action::Release && key == Key::Key_Escape) {
		if (_locked) {
			_engine.system<Window>().setLockedCursor(false);
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
	case Key::Key_W:
		_forward = value;
		return;
	case Key::Key_S:
		_back = value;
		return;
	case Key::Key_A:
		_left = value;
		return;
	case Key::Key_D:
		_right = value;
		return;
	case Key::Key_Space:
		_up = value;
		return;
	case Key::Key_LCtrl:
		_down = value;
		return;
	case Key::Key_LShift:
		_boost = value;
		return;
	}
}

void Controller::cursorEnter(bool enterered) {
	_cursorInside = enterered;
}

void Controller::mousePress(uint32_t button, Action action, uint8_t mods) {
	if (action == Action::Release && !_locked && _cursorInside) {
		_engine.system<Window>().setLockedCursor(true);
		_locked = true;
	}
}

/*
void Controller::windowOpen(bool opened){
	if (!opened)
		_engine.quit();
}
*/

void Controller::scrollWheel(glm::dvec2 offset){
	_flash += (float)offset.y;
}

void Controller::setPossessed(uint64_t id) {
	_possessed.set(id);
}