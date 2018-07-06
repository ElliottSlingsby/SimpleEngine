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
			Resizable = 1,
			Maximised = 2,
			Decorated = 4,
		};

		std::string title;
		uint32_t width;
		uint32_t height;
		uint8_t mode;

		uint32_t glContextVersionMajor;
		uint32_t glContextVersionMinor;
		bool glCoreContext;
		bool glDebugContext;

		uint32_t superSampling;
		bool vSync;
	};

	Window(Engine& engine);
	~Window();

	void initiate(int argc, char** argv) override;
	void update(double dt) override;

	void open(const Config& config);
	void close();

	void swapBuffer();
};