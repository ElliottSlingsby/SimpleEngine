#pragma once

#include "EventHandler.hpp"
#include "EntityManager.hpp"
#include "Utility.hpp"

#include <chrono>
#include <cstdint>
#include <vector>

template <uint32_t components, uint32_t events, uint32_t listeners>
class SimpleEngine {
	std::vector<std::optional<BasePtr>> _systems;

public:
	EventHandler<events, listeners> events;
	EntityManager<components> entities;

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

template<uint32_t components, uint32_t events, uint32_t listeners>
SimpleEngine<components, events, listeners>::SimpleEngine(size_t chunkSize) : entities(chunkSize){}

template<uint32_t components, uint32_t events, uint32_t listeners>
inline SimpleEngine<components, events, listeners>::~SimpleEngine(){
	for (auto& i : _systems) {
		if (i == std::nullopt)
			continue;

		i->~BasePtr();
	}
}

template<uint32_t components, uint32_t events, uint32_t listeners>
template<typename T, typename ...Ts>
void SimpleEngine<components, events, listeners>::newSystem(Ts && ...args) {
	uint32_t index = typeIndex<SimpleEngine, T>();

	if (_systems.size() <= index)
		_systems.resize(index + 1);

	if (!hasSystem<T>())
		new (&_systems[index]) std::optional<VirtualPtr<T>>(*this, args...);
}

template<uint32_t components, uint32_t events, uint32_t listeners>
template<typename T>
T& SimpleEngine<components, events, listeners>::system() {
	assert(hasSystem<T>() && "system does not exist");

	return *static_cast<T*>(_systems[typeIndex<SimpleEngine, T>()].value().ptr);
}

template<uint32_t components, uint32_t events, uint32_t listeners>
template<typename T>
bool SimpleEngine<components, events, listeners>::hasSystem() {
	uint32_t index = typeIndex<SimpleEngine, T>();

	if (_systems.size() <= index)
		return false;

	return _systems[index] != std::nullopt;
}
