#include "SystemInterface.hpp"

#include "Transform.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	Window::ConstructorInfo windowInfo;
	Renderer::ConstructorInfo rendererInfo;

	engine.registerSystem<Window>(engine, windowInfo);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Renderer>(engine, rendererInfo);

	SYSFUNC_CALL(SystemInterface, initiate, engine)(std::vector<std::string>(argv, argv + argc));
	
	// Testing state
	{
		const std::string path = upperPath(replace('\\', '/', argv[0])) + "data/";

		Window& window = engine.system<Window>();
		Controller& controller = engine.system<Controller>();
		Renderer& renderer = engine.system<Renderer>();

		{
			// Window setup
			Window::WindowInfo windowConfig;

			window.openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeInfo shapeInfo;
			shapeInfo.verticalFov = 90;
			shapeInfo.zDepth = 100000;

			renderer.reshape(shapeInfo);
			renderer.defaultTexture(path + "grey.png");
			renderer.defaultProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl");
		}

		// Camera
		{
			uint64_t id = engine.createEntity();

			Transform& transform = *engine.addComponent<Transform>(id);
			transform.position = { 0.f, -100.f, 100.f };
			transform.rotation = glm::quat({ glm::radians(90.f), 0.f, 0.f });

			renderer.setCamera(id);
			controller.setPossessed(id);
		}

		// Floor
		{
			uint64_t id = engine.createEntity();

			Transform& transform = *engine.addComponent<Transform>(id);
			transform.scale = { 10000.f, 10000.f, 10000.f };

			renderer.loadMesh(path + "plane.obj", id);
			renderer.loadTexture(path + "checker.png", id);
		}

		// Skybox
		{
			uint64_t id = engine.createEntity();
		
			Transform& transform = *engine.addComponent<Transform>(id);
			transform.scale = { 1000.f, 1000.f, 1000.f };
		
			renderer.loadMesh(path + "skybox.obj", id);
			renderer.loadTexture(path + "skybox.png", id);
		}
	}

	// Main loop
	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, update, engine)(dt);
		SYSFUNC_CALL(SystemInterface, lateUpdate, engine)(dt);

		dt = deltaTime(timer);
	}
	
	return 0;
}