#include <EntityManager.hpp>
#include <EventHandler.hpp>

#include <iostream>

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
using EntityManger = SimpleEngine::EntityManager<16>;

struct Vec3 {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

struct Quat {
	float w = 1.f;
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

struct Transform {
	Vec3 position;
	Vec3 scale = { 1.f, 1.f, 1.f };
	Quat rotation;
};

struct TestObject {
	int a = 4;
	bool b = false;
	char c[8] = "Hello";

	TestObject() {}

	~TestObject() {}
};

enum Events {
	Load = 0,
	Update = 1,
};

//typedef Load;

class System {
	EventHandler& _eventHandler;

public:
	System(EventHandler* eventHandler) : _eventHandler(*eventHandler) {
		_eventHandler.subscribe(this, Events::Load, &System::load);
		_eventHandler.subscribe(this, Events::Update, &System::update);
	}

	void load(int argc, char** argv) {
		//std::cout << "hello";
	}

	void update(double dt) {
		//std::cout << "hello";
	}
};

void test(std::vector<uint64_t>* ids, EventHandler* eventHandler, EntityManger* entityManager) {
	// Event dispatching
	for (uint64_t& id : *ids) {
		eventHandler->dispatch(Events::Update, 0.0);
	}

	// Creating
	for (uint64_t& id : *ids) {
		id = entityManager->create();
		entityManager->add<TestObject>(id);
		entityManager->add<Transform>(id);
	}

	// Iterating
	entityManager->iterate<TestObject, Transform>([&](uint32_t index, auto& testObject, auto& transform) {

	});

	// Erasing
	for (uint64_t& id : *ids) {
		entityManager->erase(id);
	}
}

int main(int argc, char** argv) {
	EventHandler* eventHandler = new EventHandler();
	EntityManger* entityManager = new EntityManger(1024 * 1024 * 128); // 128 MB

	entityManager->reserve<TestObject, Transform>(100000); // 1M

	System system0(eventHandler);
	System system1(eventHandler);
	System system2(eventHandler);

	std::vector<uint64_t> ids;
	ids.resize(100000);

	std::vector<double> averageArray;
	averageArray.resize(10);
	uint32_t currentAverage = 0;

	TimePoint frameTime;

	for (uint32_t i = 0; i < averageArray.size(); i++) {
		averageArray[i] = 144.0;
	}

	while (1) {
		startTime(&frameTime);

		test(&ids, eventHandler, entityManager);

		averageArray[currentAverage] = 1.0 / deltaTime(frameTime);
		currentAverage++;

		if (currentAverage >= averageArray.size())
			currentAverage = 0;

		double average = 0.0;
		for (double i : averageArray) {
			average += i;
		}

		average /= averageArray.size();

		std::cout << "Total - " << average << " fps" << std::endl;
	}

	delete entityManager;
	delete eventHandler;

	return 0;
}