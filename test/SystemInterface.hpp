#pragma once

#include <SimpleEngine.hpp>

class SystemInterface : public SimpleEngine<SystemInterface, 32>::BaseSystem {
public:
	virtual void initiate(int argc, char** argv) {}
	virtual void update(double dt) {}

	virtual void physicsUpdate(double timescale, unsigned int steps) {}

	virtual void mousePress(int button, int action, int mods) {}
	virtual void keyPress(int key, int scancode, int action, int mods) {}
	virtual void scrollWheel(double xoffset, double yoffset) {}

	virtual void fileDrop(int count, const char** paths) {}
	virtual void cursorEnter(int entered) {}
	virtual void cursorPosition(double xpos, double ypos) {}
	virtual void textInput(unsigned int codepoint, int mods) {}

	virtual void windowPosition(int xpos, int ypos) {}
	virtual void framebufferSize(int width, int height) {}
	virtual void windowSize(int width, int height) {}
	virtual void windowClose() {}
};