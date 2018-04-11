#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"
#include "Collider.hpp"

/*

- Collider features like mass, center of mass, friction, etc
- Collider shape scaling
- Collider callbacks

- Seperate Assets system for loading image and mesh data
- Triangle mesh and convex hull Colliders

- Dynamic components during iterate with system to hookup events
- Dynamic component Controller to replace system

- User defined component erase order
- Referenced entity object

- Rendering stuff

*/

class MyState {
	Engine& _engine;

	uint64_t _camera = 0;
	uint64_t _cursor = 0;

public:
	MyState(Engine& engine) : _engine(engine) {
		_engine.events.subscribe(this, Events::Load, &MyState::load);
	}

	void load(int argc, char** argv) {
		Engine::EntityManager& entities = _engine.entities;

		Renderer& renderer = _engine.system<Renderer>();
		Physics& physics = _engine.system<Physics>();
		Controller& controller = _engine.system<Controller>();

		physics.setGravity(WorldVec3::down * 400.0);
		renderer.setDefaultShader("vertexShader.glsl", "fragmentShader.glsl");

		// Skybox
		uint64_t skybox = entities.create();
		{
			Transform& transform = *entities.add<Transform>(skybox);
			transform.setScale({ 1000, 1000, 1000 });

			renderer.addMesh(skybox, "skybox.obj");
			renderer.addTexture(skybox, "skybox.png");
		}

		// Camera
		_camera = entities.create();
		{
			Transform& transform = *entities.add<Transform>(_camera);
			transform.setPosition({ 0.0, -250.0, 50.0 });
			transform.setRotation(Quat({ glm::radians(90.0), 0.0, 0.0 }));

			Collider& collider = *physics.addSphere(_camera, 4.0, 100.0);
			collider.setGravity({ 0, 0, 0 });

			renderer.setCamera(_camera);
			controller.setPossessed(_camera);
		}

		// Cursur
		_cursor = entities.create();
		{
			Transform& transform = *entities.add<Transform>(_cursor);
			transform.setScale({ .25, .25, .25 });

			renderer.addMesh(_cursor, "arrow.obj");
			renderer.addTexture(_cursor, "arrow.png");

			controller.setCursor(_cursor);
		}

		// Floor
		uint64_t floor = entities.create();
		{
			Transform& transform = *entities.add<Transform>(floor);
			transform.setScale({ 10000.0, 10000.0, 10000.0 });

			renderer.addMesh(floor, "plane.obj");
			renderer.addTexture(floor, "checker.png");

			entities.get<Model>(floor)->linearTexture = false;

			physics.addStaticPlane(floor);

			Collider& collider = *entities.get<Collider>(floor);
			collider.setFriction(10.0);
		}

		// Axis
		uint64_t axis = entities.create();
		{
			Transform& childTransform = *entities.add<Transform>(axis);
			childTransform.setScale({ 1.0, 1.0, 1.0 });

			renderer.addMesh(axis, "axis.obj");
			renderer.addTexture(axis, "rgb.png");
		}
	}
};

int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Physics>();
	engine.newSystem<Renderer>();

	engine.newSystem<MyState>();

	engine.events.dispatch(Events::Load, argc, argv);
	
	// Update
	TimePoint timer;
	double dt = 0.0;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Input);
		engine.events.dispatch(Events::Update, dt);

		dt = deltaTime(timer);
	}

	return 0;
}