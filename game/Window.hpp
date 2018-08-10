#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class Window : public SystemInterface {
	Engine& _engine;

	//GLFWwindow* _context = nullptr;
	GLFWwindow* _window = nullptr;

public:
	enum Mode : uint8_t {
		Resizable = 1,
		Decorated = 2,

		VerticalSync = 4,
		LockedCursor = 8,
		Fullscreen = 16,
	};

	struct ConstructorInfo {
		uint32_t contextVersionMajor = 4;
		uint32_t contextVersionMinor = 6;
		bool debugContext = true;
		bool coreContex = true;

		std::string defualtTitle = "Window";
		glm::uvec2 defualtSize = { 800, 600 };
		glm::vec2 defualtPosition = { 0.5, 0.5 };
		uint8_t defualtModes = Resizable | Decorated | VerticalSync;// | LockedCursor | Fullscreen;
		uint32_t defaultSuperSampling = 1;
		uint32_t defaultMonitor = 0;
		glm::uvec2 defaultResolution = { 1920, 1080 };
	};

	const ConstructorInfo _constructorInfo;

	glm::uvec2 _lastSize = _constructorInfo.defualtSize;
	glm::vec2 _lastPosition = _constructorInfo.defualtPosition;

	glm::uvec2 _resolution = _constructorInfo.defaultResolution;
	uint32_t _monitor = _constructorInfo.defaultMonitor;

	void _createWindow();

public:
	Window(Engine& engine, const ConstructorInfo& constructorInfo = ConstructorInfo());
	~Window();

	void initiate(const std::vector<std::string>& args) override;
	void preUpdate(double dt) override;
	void update(double dt) override;
	//void rendered() override;

	void setModes(uint8_t modes, bool value);

	void setResolution(glm::uvec2 resolution);
	
	uint32_t getMonitorCount() const;
	glm::uvec2 getMonitorResolution() const;

	void setMonitor(uint32_t monitor);

	void setPosition(glm::vec2 position);
	void setSize(glm::vec2 size);

	//void setTitle(const std::string& title);
	//void setSize(glm::uvec2 size);
	//void setPosition(glm::vec2 position);
};