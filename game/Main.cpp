#include "SystemInterface.hpp"

#include "Transform.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"
#include "AssetLoader.hpp"

class MyGame : public SystemInterface{
	Engine& _engine;

	Engine::Entity _camera;
	Engine::Entity _test;

public:
	MyGame(Engine& engine) : _engine(engine), _camera(engine), _test(engine){
		SYSFUNC_ENABLE(SystemInterface, initiate, 0);
		SYSFUNC_ENABLE(SystemInterface, windowOpen, 0);
	}

	void initiate(const std::vector<std::string>& args) override{
		const std::string path = upperPath(replace('\\', '/', args[0])) + "data/";

		{
			// Window setup
			Window::WindowConfig windowConfig;

			windowConfig.windowTitle = "My Window";
			windowConfig.windowSize = { 512, 512 };
			windowConfig.flags |= Window::WindowConfig::WindowDecorated | Window::WindowConfig::WindowResizable;

			windowConfig.contextVersionMajor = 4;
			windowConfig.contextVersionMinor = 6;
			windowConfig.flags |= Window::WindowConfig::CoreContext | Window::WindowConfig::DebugContext;

			_engine.system<Window>().openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeConfig shapeConfig;

			shapeConfig.verticalFov = 90;
			shapeConfig.zDepth = 10000;

			_engine.system<Renderer>().setShape(shapeConfig);
		}

		// Camera
		{
			_camera.set(_engine.createEntity());

			Transform& transform = *_camera.add<Transform>(_engine, _camera.id());
			transform.setPosition({ 0.f, -20.f, 0.f });
			transform.setRotation({ { glm::radians(90.f), 0.f, 0.f } });

			_engine.system<Renderer>().setCamera(_camera.id());
			_engine.system<Controller>().setPossessed(_camera.id());
		}

		// Test object
		{
			_test.set(_engine.createEntity());

			_test.add<Transform>(_engine, _test.id());

			_engine.system<AssetLoader>().loadMesh(path + "dcube.obj", _test.id());
			_engine.system<AssetLoader>().loadTexture(path + "rgb.png", _test.id());
			_engine.system<Renderer>().loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl", _test.id());
		}
	}

	void windowOpen(bool opened) override {
		if (!opened)
			_engine.quit();
	}
};

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	engine.registerSystem<Window>(engine);
	engine.registerSystem<Controller>(engine);
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