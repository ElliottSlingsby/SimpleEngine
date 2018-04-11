#include "Controller.hpp"

#include "Renderer.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Collider.hpp"

uint64_t _test = 0;

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Update, &Controller::update);
	_engine.events.subscribe(this, Events::Reset, &Controller::reset);

	_engine.events.subscribe(this, Events::Mousemove, &Controller::mousemove);
	_engine.events.subscribe(this, Events::Mousepress, &Controller::mousepress);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
}

void Controller::load(int argc, char ** argv){
	_cursor = _engine.entities.create();

	Transform& transform = *_engine.entities.add<Transform>(_cursor);
	transform.setScale({ .25, .25, .25 });

	_engine.system<Renderer>().addShader(_cursor, "vertexShader.glsl", "fragmentShader.glsl");
	_engine.system<Renderer>().addMesh(_cursor, "arrow.obj");
	_engine.system<Renderer>().addTexture(_cursor, "arrow.png");

	_engine.entities.reference(_cursor);
}

void Controller::update(double dt) {
	if (!_possessed || !_engine.entities.has<Transform>(_possessed))
		return;

	Transform& transform = *_engine.entities.get<Transform>(_possessed);

	if (_locked) {
		transform.worldRotate(Quat({ 0.0, 0.0, -_dMousePos.x * dt }));
		transform.rotate(Quat({ -_dMousePos.y * dt, 0.0, 0.0 }));

		//transform.lookAt(Vec3(0.0, 0.0, 0.0));

		_dMousePos = { 0.0, 0.0 };

		double moveSpeed;

		if (_boost)
			moveSpeed = 500.0 * dt;
		else
			moveSpeed = 100.0 * dt;

		if (_forward)
			transform.translate(LocalVec3::forward * moveSpeed);
		if (_back)
			transform.translate(LocalVec3::back * moveSpeed);
		if (_left)
			transform.translate(LocalVec3::left * moveSpeed);
		if (_right)
			transform.translate(LocalVec3::right * moveSpeed);
		if (_up)
			transform.worldTranslate(WorldVec3::up * moveSpeed);
		if (_down)
			transform.worldTranslate(WorldVec3::down * moveSpeed);
	}

	Collider* collider = _engine.entities.get<Collider>(_possessed);

	if (collider) {
		Vec3 angles = glm::eulerAngles(transform.worldRotation());
		transform.setWorldRotation(Quat({ angles.x, 0.f, angles.z }));

		collider->setAngularVelocity({ 0, 0, 0 });
		collider->setLinearVelocity({ 0, 0, 0 });

		collider->activate();
	}
	
	if (_cursor && _engine.entities.has<Transform>(_cursor)) {
		Vec3 target;

		if (_action0) {
			Renderer& renderer = _engine.system<Renderer>();

			Transform& transform = *_engine.entities.get<Transform>(_possessed);

			if (!_locked) {
				Vec2 mousePos = ((_mousePos / static_cast<Vec2>(renderer.windowSize())) * 2.0) - 1.0;

				Vec4 mouseToWorld = glm::inverse(renderer.projectionMatrix() * renderer.viewMatrix()) * Vec4(mousePos.x, -mousePos.y, 0.0, 1.0);
				Vec3 cursorPosition = Vec3(mouseToWorld.x, mouseToWorld.y, mouseToWorld.z) / mouseToWorld.w;

				target = transform.position() + glm::normalize(cursorPosition - transform.position()) * 10000.0;
			}
			else {
				target = transform.position() + transform.rotation() * LocalVec3::forward * 10000.0;
			}

			auto hit = _engine.system<Physics>().rayTest(transform.position(), target);

			if (hit.id)
				target = hit.position;

			_cursorPosition = target;
		}

		_engine.entities.get<Transform>(_cursor)->setRotation(Quat({ 0.0, 0.0, glm::radians(_cursorI) }));
		_engine.entities.get<Transform>(_cursor)->setPosition(_cursorPosition + Vec3(0.0, 0.0, ((glm::cos(glm::radians(_cursorI)) + 1.0) / 2.0) * 8.0));

		_cursorI += 360.0 * dt;

		if (_cursorI >= 360.0)
			_cursorI = 0;
	}
}

void Controller::mousemove(double x, double y){
	Vec2 newMousePos = { x, y };

	if (_mousePos == Vec2(0.0, 0.0))
		_mousePos = newMousePos;

	_dMousePos = newMousePos - _mousePos;
	_mousePos = newMousePos;
}

void Controller::mousepress(int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
		_action0 = true;
	else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
		_action0 = false;
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
	if (id && !_engine.entities.valid(id))
		return;

	if (_possessed)
		_engine.entities.dereference(_possessed);

	if (id)
		_engine.entities.reference(id);
	
	_possessed = id;
}

void Controller::setCursor(uint64_t id){
	if (id && !_engine.entities.valid(id))
		return;

	if (_cursor)
		_engine.entities.dereference(_cursor);

	if (id)
		_engine.entities.reference(id);

	_cursor = id;
}
