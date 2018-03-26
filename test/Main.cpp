#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"
#include "Collider.hpp"

/*
- Implement hierarchical physics and constraints
- Raycasting and collision callbacks

- Implement generic components with pointers
- Implement referenced entity object

- Physics update and timing issues

- Gamestate system for testing

- GPU bullet
*/

class GameState {
	Engine& _engine;

	std::vector<uint64_t> _cubes;

	void _spawnCubes() {
		for (uint64_t id : _cubes) 
			_engine.entities.erase(id);

		_cubes.clear();

		for (uint32_t i = 0; i < 32; i++) {
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setPosition({ 0.0, 0.0, 8.0 + 16.0 * i });

			_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(id, "dcube.obj");
			_engine.system<Renderer>().addTexture(id, "net.png");

			_engine.system<Physics>().addBox(id, { 4.0, 4.0, 4.0 }, 100.0);

			_cubes.push_back(id);
		}
	}

public:
	GameState(Engine& engine) : _engine(engine) {
		_engine.events.subscribe(this, Events::Load, &GameState::load);
		_engine.events.subscribe(this, Events::Keypress, &GameState::keypress);
	}

	void load(int argc, char** argv) {
		// Gravity
		_engine.system<Physics>().setGravity(GlobalVec3::down * 400.f);

		// Skybox
		{
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setScale({ 500, 500, 500 });

			_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(id, "skybox.obj");
			_engine.system<Renderer>().addTexture(id, "skybox.png");
		}

		// Camera
		{
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setPosition({ 0.0, -50.0, 10.0 });
			transform.setRotation(glm::dquat({ glm::radians(90.0), 0.0, 0.0 }));

			_engine.system<Physics>().addSphere(id, 4.0, 50.0);
			Collider& collider = *_engine.entities.get<Collider>(id);
			collider.setGravity({ 0, 0, 0 });

			_engine.system<Renderer>().setCamera(id);

			_engine.system<Controller>().setPossessed(id);
		}

		// Floor
		{
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setScale({ 10000.0, 10000.0, 10000.0 });

			_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(id, "plane.obj");
			_engine.system<Renderer>().addTexture(id, "checker.png");

			_engine.entities.get<Model>(id)->linearTexture = false;

			_engine.system<Physics>().addStaticPlane(id);

			Collider& collider = *_engine.entities.get<Collider>(id);
			collider.setFriction(10.f);
		}

		// Cubes
		_spawnCubes();
	}

	void keypress(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_RELEASE)
			_spawnCubes();
	}
};

int main(int argc, char** argv) {
	// Load
	Engine engine(chunkSize);

	engine.newSystem<Controller>();
	engine.newSystem<Physics>();
	engine.newSystem<Renderer>();

	engine.newSystem<GameState>();

	engine.events.dispatch(Events::Load, argc, argv);
	
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