#pragma once

#include "Config.hpp"

#include <glm\vec2.hpp>

class Controller {
	Engine& _engine;

	uint64_t _possessed = 0;

	bool _forward = false;
	bool _back = false;
	bool _left = false;
	bool _right = false;

	bool _up = false;
	bool _down = false;

	glm::dvec2 _cursor;
	glm::dvec2 _dCursor;

	bool _locked = true;

public:
	Controller(Engine& engine);

	void update(double dt);
	void cursor(double x, double y);
	void keypress(int key, int scancode, int action, int mods);

	void setPossessed(uint64_t id);
};