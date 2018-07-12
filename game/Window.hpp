#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class Window : public SystemInterface {
	Engine& _engine;
	GLFWwindow* _window = nullptr;

public:
	struct Config {
		enum Modes : uint8_t{
			WindowResizable = 1,
			WindowMaximised = 2,
			WindowDecorated = 4,

			CoreContext = 8,
			DebugContext = 16,

			VerticalSync = 32,
		};

		std::string windowTitle = "";
		uint32_t windowWidth = 0;
		uint32_t windowHeight = 0;

		uint32_t contextVersionMajor = 1;
		uint32_t contextVersionMinor = 0;

		uint32_t superSampling = 0;

		uint8_t flags = 0;

	};

	Window(Engine& engine);
	~Window();

	void initiate(int argc, char** argv) override;
	void update(double dt) override;

	void open(const Config& config);
	void close();

	void swapBuffer();
};