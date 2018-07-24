#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class Window : public SystemInterface {
	Engine& _engine;

	GLFWwindow* _window = nullptr;

public:
	enum Mode : uint8_t {
		Resizable = 1,
		Maximised = 2,
		Decorated = 4,
		VerticalSync = 8,
		LockedCursor = 16
	};

	struct ConstructorInfo {
		uint32_t contextVersionMajor = 4;
		uint32_t contextVersionMinor = 6;
		bool debugContext = true;
		bool coreContex = true;

		std::string defualtTitle = "Window";
		glm::uvec2 defualtSize = { 800, 600 };
		glm::vec2 defualtPosition = { -1, -1 };
		uint8_t defualtModes = Resizable | Decorated | VerticalSync;
		uint32_t defaultSuperSampling = 1;
	};

	const ConstructorInfo _constructorInfo;

public:
	Window(Engine& engine, const ConstructorInfo& constructorInfo = ConstructorInfo());
	~Window();

	void initiate(const std::vector<std::string>& args) override;
	void update(double dt) override;
	void rendered() override;
	
	void enableModes(uint8_t modes);
	void disableModes(uint8_t modes);
	
	//void setTitle(const std::string& title);
	//void setSize(glm::uvec2 size);
	//void setPosition(glm::vec2 position);
};