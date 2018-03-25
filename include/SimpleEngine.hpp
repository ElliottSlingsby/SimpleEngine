#pragma once

#include "EventHandler.hpp"
#include "EntityManager.hpp"
#include "Utility.hpp"

#include <chrono>
#include <cstdint>
#include <vector>

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
class SimpleEngine {
	std::optional<BasePtr> _systems[systems];

public:
	using EventHandler = EventHandler<events, listeners>;
	using EntityManager = EntityManager<components>;

	EventHandler events;
	EntityManager entities;

	bool running = true;

	inline SimpleEngine(size_t chunkSize);

	inline ~SimpleEngine();

	template <typename T, typename ...Ts>
	inline void newSystem(Ts&&... args);

	template <typename T>
	inline T& system();

	template <typename T>
	inline bool hasSystem();
};

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
SimpleEngine<systems, components, events, listeners>::SimpleEngine(size_t chunkSize) : entities(chunkSize){}

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
inline SimpleEngine<systems, components, events, listeners>::~SimpleEngine(){
	for (uint32_t i = 0; i < systems; i++) {
		if (_systems[i] == std::nullopt)
			continue;

		_systems[i]->~BasePtr();
	}
}

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
template<typename T, typename ...Ts>
void SimpleEngine<systems, components, events, listeners>::newSystem(Ts && ...args) {
	uint32_t index = typeIndex<SimpleEngine, T>();

	assert(index < systems && "ran out of system slots");

	if (!hasSystem<T>())
		new (&_systems[index]) std::optional<VirtualPtr<T>>(*this, std::forward<Ts>(args)...);
}

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
template<typename T>
T& SimpleEngine<systems, components, events, listeners>::system() {
	assert(hasSystem<T>() && "system does not exist");

	return *static_cast<T*>(_systems[typeIndex<SimpleEngine, T>()].value().ptr);
}

template <uint32_t systems, uint32_t components, uint32_t events, uint32_t listeners>
template<typename T>
bool SimpleEngine<systems, components, events, listeners>::hasSystem() {
	uint32_t index = typeIndex<SimpleEngine, T>();

	if (index >= systems)
		return false;

	return _systems[index] != std::nullopt;
}
