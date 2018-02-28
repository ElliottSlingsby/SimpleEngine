#include <EntityManager.hpp>
#include <EventHandler.hpp>

#include <iostream>
#include <chrono>
#include <thread>

/*

- insert/erase/add/remove work during iteration
- Reference counting / entity states
- Clean up EntityManager
- Event Handler

- Main function and system handling (create entity entityManager, user constructs systems, passes references to engine and runs)

- OpenGL 4 renderer
- Bullet physics

*/



using EventHandler = SimpleEngine::EventHandler<16, 16>;

struct Vec3 {
	float x;
	float y;
	float z;
};

struct Quat {
	float w;
	float x;
	float y;
	float z;
};

struct Transform {
	Vec3 position = { 0.f, 0.f, 0.f };
	Vec3 scale = { 1.f, 1.f, 1.f };
	Quat rotation = { 1.f, 0.f, 0.f, 0.f };
};

struct Velocity {
	Vec3 position = { 0.f, 0.f, 0.f };
	Quat rotation = { 1.f, 0.f, 0.f, 0.f };
};

struct TestObject {
	int a = 4;
	bool b = false;
	char c[8] = "Hello";

	TestObject() {

	}

	~TestObject() {

	}
};

using Clock = std::chrono::high_resolution_clock;

template <typename T = double>
inline T seconds(const Clock::duration& duration) {
	return std::chrono::duration_cast<std::chrono::duration<T>>(duration).count();
}

inline void start(Clock::time_point* point) {
	*point = Clock::now();
}

inline void end(Clock::time_point* point, std::string prefix = "") {
	std::cout << prefix << seconds(Clock::now() - *point) * 1000.0 << " ms" << std::endl;
}

enum Events {
	Load,
	Update,
};

class System0 {
	EventHandler& _eventHandler;

public:
	System0(EventHandler* eventHandler) : _eventHandler(*eventHandler) {
		_eventHandler.subscribe(this, Events::Update, &System0::update);
	}

	void update(double dt) {	}
};

class System1 {
	EventHandler& _eventHandler;

public:
	System1(EventHandler* eventHandler) : _eventHandler(*eventHandler) {
		_eventHandler.subscribe(this, Events::Update, &System1::update);
	}

	void update(double dt) {	}
};

class System2 {
	EventHandler& _eventHandler;

public:
	System2(EventHandler* eventHandler) : _eventHandler(*eventHandler) {
		_eventHandler.subscribe(this, Events::Update, &System2::update);
	}

	void update(double dt) {	}
};

int main(int argc, char** argv) {
	// Event Handler
	EventHandler* eventHandler = new EventHandler();

	// Entity manager
	EntityManager<16>* entityManager = new EntityManager<16>(1024 * 1024 * 128); // 128 MB

	entityManager->reserve<TestObject, Transform>(1000000); // 1M

	System0 system0(eventHandler);
	System1 system1(eventHandler);
	System2 system2(eventHandler);

	std::vector<uint64_t> ids;
	ids.resize(1000000);

	Clock::time_point point;

	while (1) {
		// Event dispatching
		start(&point);
		
		for (uint64_t& id : ids) {
			eventHandler->dispatch(Events::Update, 0.0);
		}
		
		end(&point, "Events\t\t");
		
		// Creating
		start(&point);

		for (uint64_t& id : ids) {
			id = entityManager->create();
			entityManager->add<TestObject>(id);
			entityManager->add<Transform>(id);
		}

		end(&point, "Creating\t");

		// Iterating
		start(&point);

		entityManager->iterate<TestObject, Transform>([&](uint32_t index, auto& testObject, auto& transform) {

		});

		end(&point, "Iterating\t");

		// Erasing
		start(&point);

		for (uint64_t& id : ids) {
			entityManager->erase(id);
		}

		end(&point, "Erasing\t\t");

		std::cout << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		system("cls");		
	}

	entityManager->clear();
	return 0;
}