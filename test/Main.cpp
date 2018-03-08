#include "Config.hpp"
#include "Renderer.hpp"

#include "Transform.hpp"
#include "Model.hpp"

int main(int argc, char** argv) {
	// load
	Engine engine(chunkSize);
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	// test entity
	uint64_t id = engine.entities.create();
	engine.entities.add<Transform>(id);

	Transform& transform = *engine.entities.get<Transform>(id);
	transform.position = { 0.f, 0.f, 10.f };
	
	if (engine.hasSystem<Renderer>()) {
		engine.system<Renderer>().loadMesh(&id, "cube.obj");
		engine.system<Renderer>().loadTexture(&id, "image.png");
	}

	// update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		// test update
		//transform.position += glm::vec3( 0.f, 0.f, -5.f * dt );
		transform.rotation *= glm::quat({ 0.f, glm::radians(360.f * dt), 0.f });

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}