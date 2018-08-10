#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>
#include <SDL.h>

class Window : public SystemInterface {
	Engine& _engine;

	SDL_Window* _window = nullptr;
	SDL_GLContext _context = nullptr;

	static const std::unordered_map<uint32_t, uint32_t> _keymap;

public:
	enum Mode {
		Undecorated,
		Fixed,
		Resizable,
		Fullscreen,
		WindowFullscreen,
		Hidden
	};

	struct ConstructorInfo {
		uint32_t contextVersionMajor = 4;
		uint32_t contextVersionMinor = 6;
		bool debugContext = true;
		bool coreContex = true;
	};

	struct WindowInfo {
		Mode mode = Resizable;
		std::string title = "";
		glm::uvec2 size = { 800, 600 };
		glm::uvec2 resolution = { 1920, 1080 };
		uint32_t monitor = 0;
		bool lockedCursor = false;
	};

private:
	const ConstructorInfo _constructorInfo;
	WindowInfo _windowInfo;

	void _recreateWindow();

public:
	Window(Engine& engine, const ConstructorInfo& constructorInfo = ConstructorInfo());
	~Window();

	void initiate(const std::vector<std::string>& args) override;
	void preUpdate(double dt) override;
	void update(double dt) override;

	void openWindow(const WindowInfo& windowInfo);
	void openWindow();
	void closeWindow();

	void setMode(Mode mode);

	void setSize(glm::uvec2 size);

	void setResolution(glm::uvec2 resolution);

	void setLockedCursor(bool locked);

	void setMonitor(uint32_t monitor);
	uint32_t getMonitorCount() const;
	glm::uvec2 getMonitorResolution(uint32_t monitor) const;
};