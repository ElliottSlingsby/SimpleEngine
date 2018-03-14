#pragma once

#include "Config.hpp"

class Controller {
	Engine& _engine;

	uint64_t _possessed = 0;

public:
	Controller(Engine& engine);

	void load(int argc, char** argv);
	void keypress(int key, int scancode, int action, int mods);

	void setPossessed(uint64_t id);
};