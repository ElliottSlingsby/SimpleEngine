#pragma once

#include "SystemInterface.hpp"

class Controller : public SystemInterface{
	Engine& _engine;

	Engine::Entity _possessed;
	Engine::Entity _cursor;

	bool _forward = false;
	bool _back = false;
	bool _left = false;
	bool _right = false;

	bool _up = false;
	bool _down = false;

	bool _boost = false;

	bool _action0 = false;

	glm::vec2 _mousePos;
	glm::vec2 _dMousePos;

	bool _locked = false;
	bool _cursorInside;

public:
	Controller(Engine& engine);

	void update(double dt) override;

	void cursorPosition(glm::dvec2 position) override;
	void keyInput(uint32_t key, Action action, Modifier mods) override;
	void cursorEnter(bool enterered) override;
	void mousePress(uint32_t button, Action action, Modifier mods) override;

	void setPossessed(uint64_t id);

	friend class EventManager;
};