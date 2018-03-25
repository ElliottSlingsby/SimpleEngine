#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"
#include "Collider.hpp"

/*
- Implement hierarchical physics and constraints
- Integrate collider into transform
- Abstract collider and transform component

- Implement generic components with pointers

- GPU bullet
*/

uint64_t createCube(Engine& engine, glm::dvec3 position) {
	uint64_t id = engine.entities.create();
	Transform& transform = *engine.entities.add<Transform>(id);

	transform.position = position;

	engine.system<Renderer>().addShader(&id, "vertexShader.glsl", "fragmentShader.glsl");
	engine.system<Renderer>().addMesh(&id, "dcube.obj");
	engine.system<Renderer>().addTexture(&id, "net.png");

	engine.system<Physics>().addBox(id, { 4.f, 4.f, 4.f }, 100.f);

	Collider& collider = *engine.entities.get<Collider>(id);
	collider.rigidBody->setFriction(100.f);

	return id;
}

uint64_t createFloor(Engine& engine, glm::dvec3 position, glm::vec3 rotation) {
	uint64_t id = engine.entities.create();
	Transform& transform = *engine.entities.add<Transform>(id);

	transform.position = position;
	transform.rotation = glm::quat(glm::radians(rotation));
	transform.scale = { 10000.f, 10000.f, 10000.f };

	engine.system<Renderer>().addShader(&id, "vertexShader.glsl", "fragmentShader.glsl");
	engine.system<Renderer>().addMesh(&id, "plane.obj");
	engine.system<Renderer>().addTexture(&id, "checker.png");

	engine.entities.get<Model>(id)->linearTexture = false;

	engine.system<Physics>().addStaticPlane(id);

	return id;
}

uint64_t createArrow(Engine& engine, glm::dvec3 position, glm::vec3 rotation, uint64_t parent) {
	uint64_t id = engine.entities.create();
	Transform& transform = *engine.entities.add<Transform>(id);

	transform.position = position;
	transform.rotation = glm::quat(glm::radians(rotation));

	if (parent)
		transform.setParent(engine.entities.get<Transform>(parent));

	engine.system<Renderer>().addShader(&id, "vertexShader.glsl", "fragmentShader.glsl");
	engine.system<Renderer>().addMesh(&id, "axis.obj");
	engine.system<Renderer>().addTexture(&id, "rgb.png");

	return id;
}

int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Physics>();
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);
	
	engine.system<Physics>().setGravity({ 0.f, 0.f, -100.f });

	// Create camera
	{
		uint64_t cameraId = engine.entities.create();
		Transform& transform = *engine.entities.add<Transform>(cameraId);

		transform.position = { 0.f, -200.f, 200.f };
		transform.rotation *= glm::quat({ glm::radians(90.f), 0.f, 0.f });

		engine.system<Renderer>().setCamera(cameraId);
		engine.system<Controller>().setPossessed(cameraId);

		engine.system<Physics>().addSphere(cameraId, 4.f, 50.f);

		Collider& collider = *engine.entities.get<Collider>(cameraId);
		collider.rigidBody->setGravity(btVector3(0, 0, 0));
	}

	// Test scene
	{
		createFloor(engine, { 0.0, 0.0, -10.0 }, { 0.f, 0.f, 0.f });

		for (uint32_t i = 0; i < 10; i++) 
			createCube(engine, { 0.0, 100.0, i * 16.0 });
	}

	// Assimp test
	{
		uint64_t fbxId = engine.entities.create();
		Transform& transform = *engine.entities.add<Transform>(fbxId);

		engine.system<Renderer>().addScene(&fbxId, "hand.FBX");
	}

	// Hierarchy test
	{
		//uint64_t a = createArrow(engine, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 45.f }, 0);

		//uint64_t b = createArrow(engine, { 0.f, 0.f, 100.f }, { -45.f, 0.f, 0.f }, a);

		//uint64_t c = createArrow(engine, { 0.f, 0.f, 100.f }, { -45.f, 0.f, 0.f }, b);

	}
	
	// Update
	TimePoint timer;
	double dt = 0.0;

	const double interval = 5.0;
	double i = interval;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Update, dt);

		i += dt;

		if (i >= interval) {
			i = 0.0;
			std::cout << 1.0 / dt << std::endl;
		}

		dt = deltaTime(timer);
	}

	return 0;
}