#include "SystemInterface.hpp"

#include "Window.hpp"
#include "Renderer.hpp"

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	{ // Initiating
		engine.registerSystem<Window>(engine);
		engine.registerSystem<Renderer>(engine);

		SYSFUNC_CALL(SystemInterface, initiate, engine)(argc, argv);
	}

	{
		Window::Config config;

		config.title = "GLFW Window";
		config.width = 512;
		config.height = 512;
		config.mode = Window::Config::Decorated | Window::Config::Resizable;

		config.glContextVersionMajor = 4;
		config.glContextVersionMinor = 6;
		config.glCoreContext = true;
		config.glDebugContext = true;

		config.superSampling = 0;
		config.vSync = false;

		engine.system<Window>().open(config);

		uint32_t program = engine.system<Renderer>().createProgram({
			{ Renderer::VertexShader, "vertexShader.glsl" },
			{ Renderer::FragmentShader, "fragmentShader.glsl" },
		});
	}

	{ // Update loop
		TimePoint timer;
		double dt = 0.0;

		while (engine.running()) {
			startTime(&timer);

			SYSFUNC_CALL(SystemInterface, update, engine)(dt);

			dt = deltaTime(timer);
		}
	}

	return 0;
}