#include "Config.hpp"

#include "Renderer.hpp"
#include "Controller.hpp"
#include "Physics.hpp"

#include "Transform.hpp"
#include "Model.hpp"
#include "Collider.hpp"

/*
- Implement hierarchical physics and constraints
- Collision callbacks

- Implement referenced entity object

- Physics proportions
- Physics update and timing issues
*/

class MyState {
	Engine& _engine;

	uint64_t _camera = 0;
	std::vector<uint64_t> _cubes;

	void _spawnCubes(glm::dvec3 position = { 0.0, 0.0, 0.0 }, double zRotation = 0.0) {
		for (uint32_t i = 0; i < 1000; i++) {
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setPosition(position + glm::dvec3(0.0, 0.0, 8.0 + 16.0 * i));
			transform.setRotation(glm::dquat({ 0.0, 0.0, zRotation }));

			_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(id, "dcube.obj");
			_engine.system<Renderer>().addTexture(id, "net.png");

			_engine.system<Physics>().addBox(id, { 4.0, 4.0, 4.0 }, 1.0);
			Collider& collider = *_engine.entities.get<Collider>(id);
			collider.deactivate();

			_cubes.push_back(id);
		}
	}

	void _spawnDomino(glm::dvec3 position = { 0.0, 0.0, 0.0 }, double zRotation = 0.0) {
		uint64_t id = _engine.entities.create();

		Transform& transform = *_engine.entities.add<Transform>(id);
		transform.setPosition(glm::dvec3(0.0, 0.0, 26.0) + position);
		transform.setRotation(glm::dquat({ 0.0, 0.0, zRotation }));

		_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
		_engine.system<Renderer>().addMesh(id, "domino.obj");
		_engine.system<Renderer>().addTexture(id, "domino.png");

		_engine.system<Physics>().addBox(id, { 6.0, 2.5, 13.0 }, 100.0);

		_cubes.push_back(id);
	}

public:
	MyState(Engine& engine) : _engine(engine) {
		_engine.events.subscribe(this, Events::Load, &MyState::load);
		_engine.events.subscribe(this, Events::Keypress, &MyState::keypress);
	}

	void load(int argc, char** argv) {
		// Gravity
		_engine.system<Physics>().setGravity(GlobalVec3::down * 400.f);

		// Skybox
		{
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setScale({ 1000, 1000, 1000 });

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

			_engine.system<Physics>().addSphere(id, 4.0, 100.0);
			Collider& collider = *_engine.entities.get<Collider>(id);
			collider.setGravity({ 0, 0, 0 });

			_engine.system<Renderer>().setCamera(id);
			_engine.system<Controller>().setPossessed(id);

			_camera = id;
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
	}

	void keypress(int key, int scancode, int action, int mods) {
		if (action != GLFW_RELEASE)
			return;

		if (key == GLFW_KEY_3) {
			Transform& transform = *_engine.entities.get<Transform>(_camera);

			glm::dvec3 offset = transform.position();
			offset.z = 0.0;

			glm::dvec3 angles = glm::eulerAngles(transform.rotation());
		
			_spawnCubes(offset, angles.z);
		}
		if (key == GLFW_KEY_1) {
			for (uint64_t id : _cubes)
				_engine.entities.erase(id);

			_cubes.clear();
		}
		else if (key == GLFW_KEY_2) {
			if (_engine.entities.has<Collider>(_camera)) {
				_engine.entities.remove<Collider>(_camera);
			}
			else {
				_engine.system<Physics>().addSphere(_camera, 4.0, 50.0);
				Collider& collider = *_engine.entities.get<Collider>(_camera);
				collider.setGravity({ 0, 0, 0 });
			}
		}
		else if (key == GLFW_KEY_4) {
			Transform& transform = *_engine.entities.get<Transform>(_camera);

			glm::dvec3 offset = transform.position();
			offset.z = 0.0;

			glm::dvec3 angles = glm::eulerAngles(transform.rotation());
			_spawnDomino(offset, angles.z);
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