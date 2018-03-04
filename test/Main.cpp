#include "Config.hpp"
#include "Renderer.hpp"

int main(int argc, char** argv) {
	Engine engine(chunkSize);

	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}