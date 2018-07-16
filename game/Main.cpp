#include "SystemInterface.hpp"

#include "Window.hpp"
#include "Renderer.hpp"

#include "AssetLoader.hpp"

class MyGame : public SystemInterface{
	Engine& _engine;

public:
	MyGame(Engine& engine) : _engine(engine){
		SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	}

	void initiate(const std::vector<std::string>& args) override{
		{
			// Window setup
			Window::WindowConfig windowConfig;

			windowConfig.windowTitle = "GLFW Window";
			windowConfig.windowSize = { 512, 512 };
			windowConfig.flags |= Window::WindowConfig::WindowDecorated | Window::WindowConfig::WindowResizable;

			windowConfig.contextVersionMajor = 4;
			windowConfig.contextVersionMinor = 6;
			windowConfig.flags |= Window::WindowConfig::CoreContext | Window::WindowConfig::DebugContext;

			_engine.system<Window>().openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeConfig shapeConfig;

			shapeConfig.resolution = windowConfig.windowSize;
			shapeConfig.verticalFov = 90;
			shapeConfig.zDepth = 10000;

			_engine.system<Renderer>().setShape(shapeConfig);

			// AssetLoader setup
			_engine.system<AssetLoader>().setFolder("data");
		}
		
		// Load test
		{
			uint64_t entity0 = _engine.createEntity();

			_engine.system<Renderer>().loadProgram({
					{ GL_VERTEX_SHADER, "vertexShader.glsl" },
					{ GL_FRAGMENT_SHADER, "fragmentShader.glsl" }
				},
				entity0
			);

			_engine.system<AssetLoader>().loadMesh("dcube.obj", entity0);
			_engine.system<AssetLoader>().loadTexture("rgb.png", entity0);
		}
	}
};

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	engine.registerSystem<Window>(engine);
	engine.registerSystem<Renderer>(engine);
	engine.registerSystem<AssetLoader>(engine);

	engine.registerSystem<MyGame>(engine);

	SYSFUNC_CALL(SystemInterface, initiate, engine)(std::vector<std::string>(argv, argv + argc));
	
	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, update, engine)(dt);

		dt = deltaTime(timer);
	}
	
	return 0;
}