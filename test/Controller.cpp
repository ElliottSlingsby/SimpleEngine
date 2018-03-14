#include "Controller.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"

Controller::Controller(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Load, &Controller::load);
	_engine.events.subscribe(this, Events::Keypress, &Controller::keypress);
}

void Controller::load(int argc, char** argv) {

}

void Controller::keypress(int key, int scancode, int action, int mods) {
	if (!_possessed)
		return;

	Transform& transform = *_engine.entities.get<Transform>(_possessed);
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
