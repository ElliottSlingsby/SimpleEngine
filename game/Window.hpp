#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class Window : public SystemInterface {
	Engine& _engine;
	GLFWwindow* _window = nullptr;

public:
	struct WindowConfig {
		enum Modes : uint8_t{
			WindowResizable = 1,
			WindowMaximised = 2,
			WindowDecorated = 4,

			CoreContext = 8,
			DebugContext = 16,

			VerticalSync = 32,
		};

		std::string windowTitle = "";
		glm::uvec2 windowSize;

		uint32_t contextVersionMajor = 1;
		uint32_t contextVersionMinor = 0;

		uint32_t superSampling = 0;

		uint8_t flags = 0;

	};

	Window(Engine& engine);
	~Window();

	void initiate(const std::vector<std::string>& args) override;
	void update(double dt) override;
	void rendered() override;

	void openWindow(const WindowConfig& config);
	void closeWindow();
};