#include "SystemInterface.hpp"

#include "Transform.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"
#include "AssetLoader.hpp"

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
			Window::WindowInfo windowConfig;

			_engine.system<Window>().openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeInfo shapeInfo;
			shapeInfo.verticalFov = 90;
			shapeInfo.zDepth = 100000;

			_engine.system<Renderer>().reshape(shapeInfo);
		}

		// Camera
		{
			_camera.create();

			Transform& transform = *_camera.add<Transform>();
			transform.setPosition({ 0.f, -100.f, 100.f });
			transform.setRotation({ { glm::radians(90.f), 0.f, 0.f } });

			_engine.system<Renderer>().setCamera(_camera);
			_engine.system<Controller>().setPossessed(_camera);
		}

		// Floor
		{
			_floor.create();
			
			Transform& transform = *_floor.add<Transform>();
			transform.setScale({ 10000.f, 10000.f, 10000.f });

			_engine.system<AssetLoader>().loadMesh(path + "plane.obj", _floor);
			_engine.system<AssetLoader>().loadTexture(path + "checker.png", _floor);
			_engine.system<Renderer>().loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl", _floor);
		}

		// Skybox
		{
			_skybox.create();

			Transform& transform = *_skybox.add<Transform>();
			transform.setScale({ 1000.f, 1000.f, 1000.f });

			_engine.system<AssetLoader>().loadMesh(path + "skybox.obj", _skybox);
			_engine.system<AssetLoader>().loadTexture(path + "skybox.png", _skybox);
			_engine.system<Renderer>().loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl", _skybox);
		}
	}
};

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	Window::ConstructorInfo windowInfo;
	Renderer::ConstructorInfo rendererInfo;

	engine.registerSystem<Window>(engine, windowInfo);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Renderer>(engine, rendererInfo);
	engine.registerSystem<AssetLoader>(engine);

	engine.registerSystem<MyGame>(engine);

	SYSFUNC_CALL(SystemInterface, initiate, engine)(std::vector<std::string>(argv, argv + argc));
	
	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, preUpdate, engine)(dt);
		SYSFUNC_CALL(SystemInterface, update, engine)(dt);

		dt = deltaTime(timer);
	}
	
	return 0;
}