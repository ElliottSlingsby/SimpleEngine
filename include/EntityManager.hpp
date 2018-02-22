#pragma once

#include "ObjectPool.hpp"
#include "TypeMask.hpp"
#include "Utility.hpp"

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <tuple>

template <uint32_t typeWidth>
class EntityManager {
	using TypeMask = TypeMask<typeWidth>;

	struct Identity {
		enum Flags : uint8_t {
			None = 0x0,
			Active = 0x1,
			Enabled = 0x2,
		};

		uint32_t index;
		uint32_t version;
		TypeMask mask;

		uint8_t flags;
	};

	const size_t _chunkSize;

	BasePool* _pools[typeWidth] = { nullptr };

	std::vector<Identity> _identities;
	std::vector<uint32_t> _freeIndexes;

	void _splitId(uint64_t id, uint32_t* index, uint32_t* version) const;

	void _eraseIndex(uint32_t index);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if<i == sizeof...(Ts), void>::type _getComponents(uint32_t index, std::tuple<Ts*...>& tuple);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if < i < sizeof...(Ts), void>::type _getComponents(uint32_t index, std::tuple<Ts*...>& tuple);

public:
	inline EntityManager(size_t chunkSize);

	inline ~EntityManager();

	inline uint64_t create();

	inline void erase(uint64_t id);

	template <typename T>
	inline T& get(uint64_t id);

	template <typename T, typename ...Ts>
	inline void add(uint64_t id, Ts... args);

	template <typename T>
	inline void remove(uint64_t id);

	template <typename ...T>
	inline bool has(uint64_t id) const;

	inline void clear();

	inline uint32_t count() const;

	template <typename ...Ts, typename T>
	inline void iterate(T lambda);
};

template <uint32_t typeWidth>
void EntityManager<typeWidth>::_splitId(uint64_t id, uint32_t* index, uint32_t* version) const {
	assert(index && version);

	*index = front64(id);
	*version = back64(id);

	assert(*index < _identities.size() && "entity id out of range");
	assert(*version == _identities[*index].version && "entity id was erased");
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::_eraseIndex(uint32_t index) {
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity

	for (uint32_t i = 0; i < typeWidth; i++) {
		if (_identities[index].mask.has(i)) {
			assert(_pools[i]); // sanity
			_pools[i]->erase(index);
		}
	}

	_identities[index].mask.clear();
	_identities[index].version++;
	_identities[index].flags = 0;

	_freeIndexes.push_back(index);
}

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i == sizeof...(Ts), void>::type EntityManager<typeWidth>::_getComponents(uint32_t index, std::tuple<Ts*...>& tuple) { }

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i < sizeof...(Ts), void>::type EntityManager<typeWidth>::_getComponents(uint32_t index, std::tuple<Ts*...>& tuple) {
	using T = std::tuple_element<i, std::tuple<Ts...>>::type;

	std::get<i>(tuple) = _pools[TypeMask::index<T>()]->get<T>(index);

	_getComponents<i + 1>(index, tuple);
}

template<uint32_t typeWidth>
EntityManager<typeWidth>::EntityManager(size_t chunkSize) : _chunkSize(chunkSize) { }

template <uint32_t typeWidth>
EntityManager<typeWidth>::~EntityManager() {
	clear();

	for (uint32_t i = 0; i < typeWidth; i++) {
		if (_pools[i])
			delete _pools[i];
	}
}

template <uint32_t typeWidth>
uint64_t EntityManager<typeWidth>::create() {
	uint32_t index;

	if (_freeIndexes.empty()) {
		index = static_cast<uint32_t>(_identities.size());
		_identities.push_back({ index, 1 });
	}
	else {
		index = *_freeIndexes.rbegin();
		_freeIndexes.pop_back();
	}

	assert(!hasFlags(_identities[index].flags, Identity::Active)); // sanity
	_identities[index].flags = Identity::Active | Identity::Enabled;

	return combine32(index, _identities[index].version);
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::erase(uint64_t id) {
	uint32_t index;
	uint32_t version;

	_splitId(id, &index, &version);

	_eraseIndex(index);
}

template <uint32_t typeWidth>
template <typename T>
T& EntityManager<typeWidth>::get(uint64_t id) {
	uint32_t index;
	uint32_t version;

	_splitId(id, &index, &version);

	assert(_pools[TypeMask::index<T>()] != nullptr); // sanity
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity
	assert(_identities[index].mask.has<T>() && "entity component non-existent");

	return *_pools[TypeMask::index<T>()]->get<T>(index);
}

template <uint32_t typeWidth>
template <typename T, typename ...Ts>
void EntityManager<typeWidth>::add(uint64_t id, Ts... args) {
	uint32_t index;
	uint32_t version;

	_splitId(id, &index, &version);

	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity
	assert(!_identities[index].mask.has<T>() && "entity component already exists");

	if (_pools[TypeMask::index<T>()] == nullptr)
		_pools[TypeMask::index<T>()] = new ObjectPool<T>(_chunkSize);

	_pools[TypeMask::index<T>()]->insert<T>(index, args...);

	_identities[index].mask.add<T>();
}

template <uint32_t typeWidth>
template <typename T>
void EntityManager<typeWidth>::remove(uint64_t id) {
	uint32_t index;
	uint32_t version;

	_splitId(id, &index, &version);

	assert(_pools[TypeMask::index<T>()] != nullptr); // sanity
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity
	assert(_identities[index].mask.has<T>() && "entity component non-existent");	

	_pools[TypeMask::index<T>()]->erase(index);

	_identities[index].mask.sub<T>();
}

template <uint32_t typeWidth>
template <typename ...T>
bool EntityManager<typeWidth>::has(uint64_t id) const {
	uint32_t index;
	uint32_t version;

	_splitId(id, &index, &version);

	if (!hasFlags(_identities[index].flags, Identity::Active))
		return false;

	return _identities[index].mask.has<T...>();
}

template<uint32_t typeWidth>
void EntityManager<typeWidth>::clear() {
	for (uint32_t i = 0; i < _identities.size(); i++) {
		if (_identities[i].mask.empty() || !hasFlags(_identities[i].flags, Identity::Active))
			continue;

		_eraseIndex(i);
	}
}

template <uint32_t typeWidth>
uint32_t EntityManager<typeWidth>::count() const {
	return _identities.size();
}

template <uint32_t typeWidth>
template <typename ...Ts, typename T>
void EntityManager<typeWidth>::iterate(T lambda) {
	std::tuple<Ts*...> components;

	for (const Identity& i : _identities) {
		if (!hasFlags(i.flags, Identity::Active | Identity::Enabled) || !i.mask.has<Ts...>())
			continue;

		_getComponents<0>(i.index, components);

		lambda(i.index, *std::get<Ts*>(components)...);
	}
}