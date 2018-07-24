#include "SystemInterface.hpp"

#include "Transform.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"
#include "AssetLoader.hpp"
#include "Physics.hpp"

class MyGame : public SystemInterface{
	Engine& _engine;

	Engine::Entity _camera;
	Engine::Entity _floor;
	Engine::Entity _skybox;

public:
	MyGame(Engine& engine) : _engine(engine), _camera(engine), _floor(engine), _skybox(engine){
		SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	}

	void initiate(const std::vector<std::string>& args) override{
		const std::string path = upperPath(replace('\\', '/', args[0])) + "data/";

		{
			// Window setup
			//Window::WindowConfig windowConfig;
			//
			//windowConfig.windowTitle = "My Window";
			//windowConfig.windowSize = { 800, 600 };
			//windowConfig.flags |= Window::WindowConfig::WindowDecorated | Window::WindowConfig::WindowResizable;
			//
			//windowConfig.contextVersionMajor = 4;
			//windowConfig.contextVersionMinor = 6;
			//windowConfig.flags |= Window::WindowConfig::CoreContext | Window::WindowConfig::DebugContext;
			//
			//_engine.system<Window>().openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeConfig shapeConfig;

			shapeConfig.verticalFov = 90;
			shapeConfig.zDepth = 100000;

			_engine.system<Renderer>().setShape(shapeConfig);
		}

		// Camera
		{
			_camera.create();

			Transform& transform = *_camera.add<Transform>(_engine, _camera.id());
			transform.setPosition({ 0.f, -100.f, 100.f });
			transform.setRotation({ { glm::radians(90.f), 0.f, 0.f } });

			_engine.system<Renderer>().setCamera(_camera.id());
			_engine.system<Controller>().setPossessed(_camera.id());
		}

		// Floor
		{
			_floor.create();
			
			Transform& transform = *_floor.add<Transform>(_engine, _floor.id());
			transform.setScale({ 10000.f, 10000.f, 10000.f });
			
			_engine.system<AssetLoader>().loadMesh(path + "plane.obj", _floor.id());
			_engine.system<AssetLoader>().loadTexture(path + "checker.png", _floor.id());
			_engine.system<Renderer>().loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl", _floor.id());
		}

		// Skybox
		{
			_skybox.create();

			Transform& transform = *_skybox.add<Transform>(_engine, _skybox.id());
			transform.setScale({ 1000.f, 1000.f, 1000.f });

			_engine.system<AssetLoader>().loadMesh(path + "skybox.obj", _skybox.id());
			_engine.system<AssetLoader>().loadTexture(path + "skybox.png", _skybox.id());
			_engine.system<Renderer>().loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl", _skybox.id());
		}
	}
};

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	engine.registerSystem<Window>(engine);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Physics>(engine);
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