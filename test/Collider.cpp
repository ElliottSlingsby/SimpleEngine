#include "Collider.hpp"

#include "Transform.hpp"

Collider::Collider(Engine::EntityManager& entities, uint64_t id) : Component(entities, id) {
	if (_engine.entities.has<Transform>(_id))
		_engine.entities.get<Transform>(id)->_collider = this;
}

Collider::~Collider(){
	if (_engine.entities.has<Transform>(_id))
		_engine.entities.get<Transform>(_id)->_collider = nullptr;
}

