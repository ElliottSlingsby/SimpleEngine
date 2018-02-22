#include <EntityManager.hpp>

#include <iostream>

struct Transform {
	uint8_t size[4];
};

struct Velocity {
	uint8_t size[8];
};

struct Model {
	uint8_t size[16];
};

struct Collider {
	uint8_t size[32];
};

class TestObject {
	int a;
	bool b;
	char c[8];

public:
	TestObject() {
		a = 4;
		b = false;
		strcpy_s(c, "Hello");

		std::cout << "Morning?" << std::endl;
	}

	~TestObject() {
		std::cout << "Goodnight..." << std::endl;
	}
};

int main(int argc, char** argv) {
	EntityManager<16>* manager = new EntityManager<16>(16000);

	std::vector<uint64_t> ids;
	ids.resize(8);

	for (uint64_t& id : ids) {
		id = manager->create();
		manager->add<TestObject>(id);
		manager->add<Transform>(id);
	}
	
	manager->iterate<TestObject, Transform>([](uint32_t index, TestObject& testObject, Transform& transform) {
		std::cout << "Evening!" << std::endl;
	});

	manager->clear();
	std::cin.get();

	return 0;
}