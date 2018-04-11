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
	uint64_t _test = 0;
	uint64_t _test1 = 0;

	std::vector<uint64_t> _junk;

	void _spawnCubes(glm::dvec3 position = { 0.0, 0.0, 0.0 }, double zRotation = 0.0) {
		for (uint32_t i = 0; i < 100; i++) {
			uint64_t id = _engine.entities.create();

			Transform& transform = *_engine.entities.add<Transform>(id);
			transform.setPosition(position + glm::dvec3(0.0, 0.0, 8.0 + 16.0 * i));
			transform.setRotation(glm::dquat({ 0.0, 0.0, zRotation }));

			_engine.system<Renderer>().addShader(id, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(id, "dcube.obj");
			_engine.system<Renderer>().addTexture(id, "net.png");

			_engine.system<Physics>().addBox(id, { 4.0, 4.0, 4.0 }, 10.0);
			Collider& collider = *_engine.entities.get<Collider>(id);
			collider.deactivate();

			_junk.push_back(id);
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

		_engine.system<Physics>().addBox(id, { 6.0, 2.5, 13.0 }, 10.0);

		_junk.push_back(id);
	}

	uint64_t _spawnParentTest(const glm::dvec3 position = { 0.0, 0.0, 0.0 }) {
		uint64_t parent = _engine.entities.create();
		{
			Transform& parentTransform = *_engine.entities.add<Transform>(parent);
			parentTransform.setPosition(position);
			parentTransform.setScale({ 30, 30, 10 });
			parentTransform.setRotation({ { 0.0, glm::radians(45.0), glm::radians(45.0) } });

			_engine.system<Renderer>().addShader(parent, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(parent, "cube.obj");
			_engine.system<Renderer>().addTexture(parent, "rgb.png");

			_engine.system<Physics>().addBox(parent, { 15, 15, 5 }, 10.f);

			_junk.push_back(parent);
		}

		uint64_t child0 = _engine.entities.create();
		{
			Transform& childTransform = *_engine.entities.add<Transform>(child0);
			childTransform.setPosition({ 0, 0, 100 });
			childTransform.setScale({ 15, 15, 15 });
			childTransform.setRotation({ { 0.0, 0.0, glm::radians(45.0) } });

			childTransform.setParent(parent);

			_engine.system<Renderer>().addShader(child0, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(child0, "cube.obj");
			_engine.system<Renderer>().addTexture(child0, "rgb.png");

			//_engine.system<Physics>().addSphere(child0, 15, 100.f);

			_engine.system<Physics>().addBox(child0, { 15 / 2, 15 / 2, 15 / 2 }, 10.f);
		}

		//_engine.entities.get<Transform>(child0)->removeParent();

		//uint64_t child1 = _engine.entities.create();
		//{
		//	Transform& childTransform = *_engine.entities.add<Transform>(child1);
		//	childTransform.setPosition({ 0, 0, 200 });
		//	childTransform.setScale({ 15, 15, 15 });
		//	//childTransform.setRotation({ { 0.0, 0.0, 45.0 } });
		//
		//	childTransform.setParent(child0);
		//
		//	_engine.system<Renderer>().addShader(child1, "vertexShader.glsl", "fragmentShader.glsl");
		//	_engine.system<Renderer>().addMesh(child1, "cube.obj");
		//	_engine.system<Renderer>().addTexture(child1, "rgb.png");
		//
		//	_engine.system<Physics>().addSphere(child1, 15, 100.f);
		//}

		//_engine.system<Controller>().setPossessed(child0);

		_engine.entities.get<Collider>(parent)->alwaysActive(true);

		_test = child0;
		_test1 = parent;

		return parent;
	}

	void _spawnAxis(const glm::dvec3& position) {
		uint64_t axis0 = _engine.entities.create();
		{
			Transform& childTransform = *_engine.entities.add<Transform>(axis0);
			childTransform.setPosition(position);
			childTransform.setScale({ 1.0, 1.0, 1.0 });

			_engine.system<Renderer>().addShader(axis0, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(axis0, "axis.obj");
			_engine.system<Renderer>().addTexture(axis0, "rgb.png");
		}
	}

public:
	MyState(Engine& engine) : _engine(engine) {
		_engine.events.subscribe(this, Events::Load, &MyState::load);
		_engine.events.subscribe(this, Events::Update, &MyState::update);
		_engine.events.subscribe(this, Events::Keypress, &MyState::keypress);
	}

	void load(int argc, char** argv) {
		//_engine.system<Physics>().setGravity(GlobalVec3::down * 400.f);
		
		// Spawn test compound entities
		for (uint32_t i = 0; i < 1; i++)
			_spawnParentTest({ 0.0, 100.0, 100.0 * i + 200.0 });

		// Skybox
		uint64_t skybox = _engine.entities.create();
		{
			Transform& transform = *_engine.entities.add<Transform>(skybox);
			transform.setScale({ 1000, 1000, 1000 });

			_engine.system<Renderer>().addShader(skybox, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(skybox, "skybox.obj");
			_engine.system<Renderer>().addTexture(skybox, "skybox.png");

		}

		// Camera
		uint64_t camera = _engine.entities.create();
		{
			Transform& transform = *_engine.entities.add<Transform>(camera);
			transform.setPosition({ 0.0, -250.0, 50.0 });
			transform.setRotation(glm::dquat({ glm::radians(90.0), 0.0, 0.0 }));

			_engine.system<Physics>().addSphere(camera, 4.0, 100.0);
			Collider& collider = *_engine.entities.get<Collider>(camera);
			collider.setGravity({ 0, 0, 0 });

			_engine.system<Renderer>().setCamera(camera);
			_engine.system<Controller>().setPossessed(camera);

			_camera = camera;
		}

		// Floor
		uint64_t floor = _engine.entities.create();
		{
			Transform& transform = *_engine.entities.add<Transform>(floor);
			transform.setScale({ 10000.0, 10000.0, 10000.0 });

			_engine.system<Renderer>().addShader(floor, "vertexShader.glsl", "fragmentShader.glsl");
			_engine.system<Renderer>().addMesh(floor, "plane.obj");
			_engine.system<Renderer>().addTexture(floor, "checker.png");

			_engine.entities.get<Model>(floor)->linearTexture = false;

			_engine.system<Physics>().addStaticPlane(floor);

			Collider& collider = *_engine.entities.get<Collider>(floor);
			collider.setFriction(10.f);
		}

		// Axis
		_spawnAxis({ 0, 100, 0 });
		_spawnAxis({ 0, 100, 50 });
		_spawnAxis({ 0, 100, 100 });
		_spawnAxis({ 0, 100, 150 });
		_spawnAxis({ 0, 100, 200 });
	}

	void update(double dt) {
		//Transform* transform = _engine.entities.get<Transform>(_test1);
		//
		//if (transform) {
		//	glm::dvec3 position = transform->worldPosition();
		//
		//	std::cout << position.x << ' ' << position.y << ' ' << position.z << '\n';
		//}

		//Transform* transform = _engine.entities.get<Transform>(_camera);
		//
		//if (transform) 
		//	transform->lookAt(_test);
	}

	void keypress(int key, int scancode, int action, int mods) {
		if (action != GLFW_RELEASE)
			return;

		if (key == GLFW_KEY_3) {
			Transform& transform = *_engine.entities.get<Transform>(_camera);

			glm::dvec3 angles = glm::eulerAngles(transform.rotation());
			_spawnCubes(_engine.system<Controller>().cursorPosition(), angles.z);
		}
		if (key == GLFW_KEY_1) {
			for (uint64_t id : _junk)
				_engine.entities.erase(id);

			_junk.clear();
		}
		else if (key == GLFW_KEY_2) {
			/*if (_engine.entities.has<Collider>(_camera)) {
				_engine.entities.remove<Collider>(_camera);
			}
			else {
				_engine.system<Physics>().addSphere(_camera, 4.0, 50.0);
				Collider& collider = *_engine.entities.get<Collider>(_camera);
				collider.setGravity({ 0, 0, 0 });
			}*/

			static bool flip = true;

			if (flip) {
				_engine.system<Controller>().setPossessed(_test);
				flip = false;
			}
			else {
				_engine.system<Controller>().setPossessed(_camera);
				flip = true;
			}

		}
		else if (key == GLFW_KEY_4) {
			Transform& transform = *_engine.entities.get<Transform>(_camera);

			glm::dvec3 angles = glm::eulerAngles(transform.rotation());
			_spawnDomino(_engine.system<Controller>().cursorPosition(), angles.z);
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

	const double counter = 1.0;
	double i = counter;

	while (engine.running) {
		startTime(&timer);

		engine.events.dispatch(Events::Input);
		engine.events.dispatch(Events::Update, dt);

		i -= dt;

		if (i <= 0.0) {
			//std::cout << 1.0 / dt << " fps" << std::endl;
			i = counter;
		}

		dt = deltaTime(timer);
	}

	return 0;
}