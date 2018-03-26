#include "Controller.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"
#include "Collider.hpp"

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Update, &Controller::update);
	_engine.events.subscribe(this, Events::Cursor, &Controller::cursor);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
	_engine.events.subscribe(this, Events::Reset, &Controller::reset);
}

void Controller::update(double dt) {
	if (!_possessed || !_engine.entities.has<Transform>(_possessed))
		return;

	Transform& transform = *_engine.entities.get<Transform>(_possessed);

	if (_locked) {
		transform.globalRotate(glm::dquat({ 0.0, 0.0, -_dCursor.x * dt }));
		transform.rotate(glm::dquat({ -_dCursor.y * dt, 0.0, 0.0 }));

		_dCursor = { 0.0, 0.0 };

		double moveSpeed;

		if (_boost)
			moveSpeed = 200.0 * dt;
		else
			moveSpeed = 100.0 * dt;

		if (_forward)
			transform.translate(LocalDVec3::forward * moveSpeed);
		if (_back)
			transform.translate(LocalDVec3::back * moveSpeed);
		if (_left)
			transform.translate(LocalDVec3::left * moveSpeed);
		if (_right)
			transform.translate(LocalDVec3::right * moveSpeed);
		if (_up)
			transform.globalTranslate(GlobalDVec3::up * moveSpeed);
		if (_down)
			transform.globalTranslate(GlobalDVec3::down * moveSpeed);
	}

	Collider* collider = _engine.entities.get<Collider>(_possessed);

	if (collider) {
		glm::dvec3 angles = glm::eulerAngles(transform.rotation());
		transform.setRotation(glm::dquat({ angles.x, 0.f, angles.z }));

		collider->setAngularVelocity({ 0, 0, 0 });
		collider->setLinearVelocity({ 0, 0, 0 });

		collider->activate();
	}
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

void Controller::reset(){
	if (_possessed)
		_engine.entities.dereference(_possessed);

	_possessed = 0;
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
