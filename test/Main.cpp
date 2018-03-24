#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"

uint64_t createCube(Engine& engine, glm::dvec3 position) {
	uint64_t id = engine.entities.create();
	engine.entities.add<Transform>(id);
	engine.entities.get<Transform>(id)->position = position;

	engine.system<Renderer>().addShader(&id, "vertexShader.glsl", "fragmentShader.glsl");
	engine.system<Renderer>().addMesh(&id, "dcube.obj");
	engine.system<Renderer>().addTexture(&id, "net.png");

	engine.system<Physics>().createRigidBody(id, { 8.f, 8.f, 8.f }, 100.f);

	return id;
}

uint64_t createFloor(Engine& engine, glm::dvec3 position, glm::vec3 rotation) {
	uint64_t id = engine.entities.create();
	engine.entities.add<Transform>(id);
	Transform& transform = *engine.entities.get<Transform>(id);

	transform.position = position;
	transform.rotation = glm::quat(glm::radians(rotation));
	transform.scale = { 1000.f, 1000.f, 1000.f };

	engine.system<Renderer>().addShader(&id, "vertexShader.glsl", "fragmentShader.glsl");
	engine.system<Renderer>().addMesh(&id, "plane.obj");
	engine.system<Renderer>().addTexture(&id, "checker.png");

	engine.system<Physics>().createRigidBody(id, { 500.f, 500.f, 1.f }, 0.f);

	return id;
}

int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Physics>();
	engine.newSystem<Renderer>();

	engine.events.dispatch(Events::Load, argc, argv);

	// Create camera
	{
		uint64_t cameraId = engine.entities.create();
		engine.entities.add<Transform>(cameraId);
		Transform& transform = *engine.entities.get<Transform>(cameraId);

		transform.position = { 0.f, -200.f, 200.f };
		transform.rotation *= glm::quat({ glm::radians(90.f), 0.f, 0.f });

		engine.system<Renderer>().setCamera(cameraId);

		engine.system<Controller>().setPossessed(cameraId);
	}

	// Assimp test
	{
		uint64_t fbxId = engine.entities.create();
		engine.entities.add<Transform>(fbxId);

		Transform& transform = *engine.entities.get<Transform>(fbxId);

		//engine.system<Renderer>().addScene(&fbxId, "hand.FBX");
	}

	///*
	// Create floors
	{
		createFloor(engine, { 0.0, 0.0, -32.0 }, { 0.f, 0.f, 0.f });
		createFloor(engine, { 100.0, 0.0, -32.0 }, { 0.f, -45.f, 0.f });
		createFloor(engine, { -100.0, 0.0, -32.0 }, { 0.f, 45.f, 0.f });

		createFloor(engine, { 0.0, 100.0, -32.0 }, { 45.f, 0.f, 0.f });
		createFloor(engine, { 0.0, -100.0, -32.0 }, { -45.f, 0.f, 0.f });
	}

	// Create cubes
	{
		for (uint32_t i = 0; i < 512; i++) 
			createCube(engine, { 0.0, 0.0, i * 16.0 });
	}
	//*/
	
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