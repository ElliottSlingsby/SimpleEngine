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

	if (engine.hasSystem<Renderer>()) {
		engine.system<Renderer>().loadMesh(&id, "quad.obj");
		engine.system<Renderer>().loadTexture(&id, "image.png");
	}

	// update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}