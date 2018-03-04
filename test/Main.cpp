#include <EventHandler.hpp>
#include <EntityManager.hpp>

#define EventHandler EventHandler<16, 16>

struct System {
	static uint32_t counter;
	const uint32_t i;

	System() : i(counter++) { }

	void function() {
		printf("Hello %d\n", i);
	}
};

uint32_t System::counter = 0;

int main(int argc, char** argv) {
	EventHandler eventHandler;

	System system0;
	System system1;
	System system2;

	eventHandler.subscribe(&system0, 0, &System::function, 0);
	eventHandler.subscribe(&system1, 0, &System::function, -2);
	eventHandler.subscribe(&system2, 0, &System::function, -1);

	eventHandler.unsubscribe(&system1);

	eventHandler.dispatch(0);

	return 0;
}