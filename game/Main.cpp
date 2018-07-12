#include "SystemInterface.hpp"

#include "Window.hpp"
#include "Renderer.hpp"

class Game : public SystemInterface{
	Engine& _engine;

public:
	Game(Engine& engine) : _engine(engine){
		SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	}

	void initiate(int argc, char** argv) {
		Window::Config config;

		config.windowTitle = "GLFW Window";
		config.windowWidth = 512;
		config.windowHeight = 512;
		config.flags |= Window::Config::WindowDecorated | Window::Config::WindowResizable;

		config.contextVersionMajor = 4;
		config.contextVersionMinor = 6;
		config.flags |= Window::Config::CoreContext | Window::Config::DebugContext;

		_engine.system<Window>().open(config);
	}
};

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	engine.registerSystem<Window>(engine);
	engine.registerSystem<Renderer>(engine);

	engine.registerSystem<Game>(engine);

	SYSFUNC_CALL(SystemInterface, initiate, engine)(argc, argv);
	
	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, update, engine)(dt);

		dt = deltaTime(timer);
	}
	
	return 0;
}