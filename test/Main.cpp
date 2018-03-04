#include <EventHandler.hpp>
#include <EntityManager.hpp>

#define EventHandler EventHandler<32, 8> // 32 events, 8 listeners
#define EntityManager EntityManager<16> // 16 components

struct System {
	static uint32_t counter;
	const uint32_t i;

	System() : i(counter++) { }

	void function(int a) {
		printf("Hello %d - %d\n", i, a);
		a += 1;
	}
};

uint32_t System::counter = 0;

int main(int argc, char** argv) {
	EventHandler eventHandler;
	EntityManager entityManager(1024 * 1024 * 128); // 128 mb chunk size

	System system0;
	System system1;
	System system2;

	eventHandler.subscribe(&system0, 0, &System::function, 0);
	eventHandler.subscribe(&system1, 0, &System::function, -2);
	eventHandler.subscribe(&system2, 0, &System::function, -1);

	eventHandler.unsubscribe(&system1);

	eventHandler.dispatch(0, 9001);

	return 0;
}