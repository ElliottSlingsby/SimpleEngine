#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"
#include "Collider.hpp"

/*
- Implement hierarchical physics and constraints

- Implement generic components with pointers
- Implement referenced entity object

- GPU bullet
*/

void load(Engine& engine) {
	engine.system<Physics>().setGravity({ 0.0, 0.0, -400.0 });

	// Camera
	{
		uint64_t id = engine.entities.create();
		Transform& transform = *engine.entities.add<Transform>(id);

		transform.setPosition({ 0.0, -50.0, 10.0 });
		transform.setRotation(glm::dquat({ glm::radians(90.0), 0.0, 0.0 }));

		engine.system<Renderer>().setCamera(id);
		engine.system<Controller>().setPossessed(id);

		engine.system<Physics>().addSphere(id, 4.0, 50.0);

		Collider& collider = *engine.entities.get<Collider>(id);
		collider.setGravity({ 0, 0, 0 });
	}

	// Floor
	{
		uint64_t id = engine.entities.create();
		Transform& transform = *engine.entities.add<Transform>(id);

		transform.setScale({ 10000.0, 10000.0, 10000.0 });

		engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
		engine.system<Renderer>().addMesh(id, "plane.obj");
		engine.system<Renderer>().addTexture(id, "checker.png");

		engine.entities.get<Model>(id)->linearTexture = false;

		engine.system<Physics>().addStaticPlane(id);

		Collider& collider = *engine.entities.get<Collider>(id);
		collider.setFriction(10.f);
	}

	// Cubes
	{
		for (uint32_t i = 0; i < 16; i++) {
			uint64_t id = engine.entities.create();
			Transform& transform = *engine.entities.add<Transform>(id);

			transform.setPosition({ 0.0, 0.0, 8.0 + 16.0 * i });

			engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			engine.system<Renderer>().addMesh(id, "dcube.obj");
			engine.system<Renderer>().addTexture(id, "net.png");

			engine.system<Physics>().addBox(id, { 4.0, 4.0, 4.0 }, 100.0);
		}
	}
}



//class GameState {
//	Engine& _engine
//public:
//
//};




int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Physics>();
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	load(engine);
	
	// Update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}