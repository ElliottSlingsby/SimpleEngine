#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"

#include "Transform.hpp"
#include "Model.hpp"

/*
- Controller for possessing entities
- Standardize matrix and UV directions
- Store filenames to prevent reloading files
*/

int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	// Create camera
	uint64_t cameraId = engine.entities.create(); {
		engine.entities.add<Transform>(cameraId);
		Transform& transform = *engine.entities.get<Transform>(cameraId);

		transform.position = { 0.f, -100.f, 0.f };
		transform.rotation *= glm::quat({ glm::radians(90.f), 0.f, 0.f });

		engine.system<Renderer>().setCamera(cameraId);
		engine.system<Controller>().setPossessed(cameraId);
	}

	// Create cube
	uint64_t cubeId = engine.entities.create(); {
		engine.entities.add<Transform>(cubeId);
		Transform& transform = *engine.entities.get<Transform>(cubeId);

		transform.position = { 10.f, 100.f, 10.f };

		engine.system<Renderer>().loadShader(&cubeId, "vertexShader.glsl", "fragmentShader.glsl");
		engine.system<Renderer>().loadMesh(&cubeId, "dcube.obj");
		engine.system<Renderer>().loadTexture(&cubeId, "net.png");
	}

	// Update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		// Spin test objects
		//engine.entities.get<Transform>(cameraId)->rotation *= glm::quat({ 0.f, 0.f, glm::radians(360.f * dt) });
		engine.entities.get<Transform>(cubeId)->rotation *= glm::quat({ 0.f, 0.f, glm::radians(360.f * dt) });

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}