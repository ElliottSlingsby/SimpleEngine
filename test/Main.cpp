#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"

#include "Transform.hpp"
#include "Model.hpp"

/*
- Controller and Renderer (key input through events)
- Seperate matrices and standardize directions (model, world, view, projection)
- Camera controls for noclip
*/

int main(int argc, char** argv) {
	// load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	// create camera
	uint64_t camera = engine.entities.create();
	engine.entities.add<Transform>(camera);

	engine.system<Renderer>().setCamera(camera);
	engine.system<Controller>().setPossessed(camera);

	// create toilet
	uint64_t toilet = engine.entities.create();
	engine.entities.add<Transform>(toilet);

	Transform& transform = *engine.entities.get<Transform>(toilet);
	transform.position = { 0.f, -19.f, 100.f };
	transform.rotation *= glm::quat({ glm::radians(90.f), 0.f, 0.f });
	
	engine.system<Renderer>().loadMesh(&toilet, "toilet1.obj");
	engine.system<Renderer>().loadTexture(&toilet, "toilet1.png");

	// update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		// test update
		//transform.position += glm::vec3( 0.f, 0.f, -1.f * dt );
		transform.rotation *= glm::quat({ 0.f, 0.f, glm::radians(360.f * dt) });

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}