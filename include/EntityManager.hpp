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
			Empty = 0x00,
			Active = 0x01,
			Enabled = 0x02,
			Erased = 0x04,
			Buffered = 0x08,
		};

		uint32_t index;
		uint32_t version;
		TypeMask mask;

		uint8_t flags = 0;
		uint32_t references = 0;
	};

	enum Warning {
		Invalid,
		References,
		Component,
	};

	const size_t _chunkSize;

	BasePool* _pools[typeWidth] = { nullptr };

	std::vector<Identity> _identities;
	std::vector<uint32_t> _freeIndexes;

	std::vector<uint32_t> _buffered;

	bool _iterating = false;

	void _warning(Warning warning, const std::string message = "") const;

	bool _validId(uint32_t index, uint32_t version) const;

	void _erase(uint32_t index);

	template <typename ...Ts, typename T>
	void _iterate(const Identity& identity, const T& lambda);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if<i == sizeof...(Ts), void>::type _get(uint32_t index, std::tuple<Ts*...>& tuple);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if < i < sizeof...(Ts), void>::type _get(uint32_t index, std::tuple<Ts*...>& tuple);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if<i == sizeof...(Ts), void>::type _reserve(uint32_t index);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if < i < sizeof...(Ts), void>::type _reserve(uint32_t index);

public:
	inline EntityManager(size_t chunkSize);

	inline ~EntityManager();

	inline bool valid(uint64_t id) const;

	inline uint64_t create();

	inline void erase(uint64_t id);

	template <typename T>
	inline T* get(uint64_t id);

	template <typename T, typename ...Ts>
	inline void add(uint64_t id, Ts&&... args);

	template <typename T>
	inline void remove(uint64_t id);

	template <typename ...Ts>
	inline bool has(uint64_t id) const;

	template <typename ...Ts>
	inline void reserve(uint32_t count);

	inline void clear();

	inline uint32_t count() const;

	template <typename ...Ts, typename T>
	inline void iterate(const T& lambda);

	inline void reference(uint64_t id);

	inline void dereference(uint64_t id);

	inline void setEnabled(uint64_t id, bool enabled);
};

template<uint32_t typeWidth>
void EntityManager<typeWidth>::_warning(Warning warning, const std::string message) const {
	switch (warning) {
	case(Warning::Invalid):
		std::cerr << "Invalid ID provided to function";
		break;
	case(Warning::References):
		std::cerr << "Problem with an ID's references";
		break;
	case(Warning::Component):
		std::cerr << "Problem with ID's component";
		break;
	}

	if (message != "")
		std::cerr << '!' << std::endl << message << '!' << std::endl;
	else
		std::cerr << std::endl;
}

template<uint32_t typeWidth>
bool EntityManager<typeWidth>::_validId(uint32_t index, uint32_t version) const{
	if (index >= _identities.size())
		return false;

	return version == _identities[index].version;
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::_erase(uint32_t index) {
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity
	
	// if references still exist, mark as erased for later, and return
	if (_identities[index].references) {
		//if (hasFlags(_identities[index].flags, Identity::Erased))
		//	_warning(Warning::References, "calling erase while entity is already erased");

		_identities[index].flags |= Identity::Erased;
		return;
	}

	// remove components from each pool
	for (uint32_t i = 0; i < typeWidth; i++) {
		if (_identities[index].mask.has(i)) {
			assert(_pools[i]); // sanity
			_pools[i]->erase(index);
		}
	}

	// clear up identity, increment version
	_identities[index].mask.clear();
	_identities[index].version++;
	_identities[index].flags = Identity::Empty;

	_freeIndexes.push_back(index);
}

template<uint32_t typeWidth>
template<typename ...Ts, typename T>
inline void EntityManager<typeWidth>::_iterate(const Identity& identity, const T& lambda){
	// skip if not active and enabled
	if (!hasFlags(identity.flags, Identity::Active | Identity::Enabled))
		return;

	// skip if erased or buffered
	if (hasFlags(identity.flags, Identity::Erased) || hasFlags(identity.flags, Identity::Buffered))
		return;

	// skip if entity doesn't have components
	if (!identity.mask.has<Ts...>())
		return;

	// get components and call lambda
	std::tuple<Ts*...> components;
	_get<0>(identity.index, components);

	lambda(combine32(identity.index, identity.version), *std::get<Ts*>(components)...);
}

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i == sizeof...(Ts), void>::type EntityManager<typeWidth>::_get(uint32_t index, std::tuple<Ts*...>& tuple) { }

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i < sizeof...(Ts), void>::type EntityManager<typeWidth>::_get(uint32_t index, std::tuple<Ts*...>& tuple) {
	using T = typename std::tuple_element<i, std::tuple<Ts...>>::type;

	std::get<i>(tuple) = _pools[typeIndex<EntityManager, T>()]->get<T>(index);

	_get<i + 1>(index, tuple);
}

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i == sizeof...(Ts), void>::type EntityManager<typeWidth>::_reserve(uint32_t index) { }

template<uint32_t typeWidth>
template <uint32_t i, typename ...Ts>
typename std::enable_if < i < sizeof...(Ts), void>::type EntityManager<typeWidth>::_reserve(uint32_t index) {
	using T = std::tuple_element<i, std::tuple<Ts...>>::type;

	if (_pools[typeIndex<EntityManager, T>()] == nullptr)
		_pools[typeIndex<EntityManager, T>()] = new ObjectPool<T>(_chunkSize);

	_pools[typeIndex<EntityManager, T>()]->reserve(index);

	_reserve<i + 1, T>(index);
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

template<uint32_t typeWidth>
inline bool EntityManager<typeWidth>::valid(uint64_t id) const {
	if (id == 0)
		return false;

	uint32_t index = front64(id);
	uint32_t version = back64(id);

	return _validId(index, version);
}

template <uint32_t typeWidth>
uint64_t EntityManager<typeWidth>::create() {
	// find free index
	uint32_t index;

	if (_freeIndexes.empty()) {
		index = static_cast<uint32_t>(_identities.size());
		_identities.push_back({ index, 1 });
	}
	else {
		index = *_freeIndexes.rbegin();
		_freeIndexes.pop_back();
	}

	// set identity flags to active, and enabled by default
	assert(!_identities[index].references); // sanity
	assert(!hasFlags(_identities[index].flags, Identity::Active)); // sanity
	_identities[index].flags = Identity::Active | Identity::Enabled;

	// if made during iteration, buffer for later
	if (_iterating) {
		_identities[index].flags |= Identity::Buffered;
		_buffered.push_back(index);
	}

	// return index and version combined
	return combine32(index, _identities[index].version);
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::erase(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling erase");
		return;
	}

	_erase(index);
}

template <uint32_t typeWidth>
template <typename T>
T* EntityManager<typeWidth>::get(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling get");
		return nullptr;
	}

	uint32_t type = typeIndex<EntityManager, T>();

	assert(_pools[type] != nullptr); // sanity
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity

	if (!_identities[index].mask.has<T>()) {
		_warning(Warning::Component, "calling get on non existant component");
		return nullptr;
	}

	return _pools[type]->get<T>(index);
}

template <uint32_t typeWidth>
template <typename T, typename ...Ts>
void EntityManager<typeWidth>::add(uint64_t id, Ts&&... args) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling add");
		return;
	}

	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity

	if (_identities[index].mask.has<T>()) {
		//_warning(Warning::Component, "calling add when component already exists");
		return;
	}

	// create pool if it doesn't exist
	if (_pools[typeIndex<EntityManager, T>()] == nullptr)
		_pools[typeIndex<EntityManager, T>()] = new ObjectPool<T>(_chunkSize);

	// remove from pool
	_pools[typeIndex<EntityManager, T>()]->insert<T>(index, args...);

	// update identity
	_identities[index].mask.add<T>();
}

template <uint32_t typeWidth>
template <typename T>
void EntityManager<typeWidth>::remove(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version))
		return;

	uint32_t type = typeIndex<EntityManager, T>();

	assert(_pools[type] != nullptr); // sanity
	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity

	if (!_identities[index].mask.has<T>()) {
		//_warning(Warning::Component, "calling remove on non existant component");
		return;
	}

	// remove from pool
	_pools[type]->erase(index);

	// update identity
	_identities[index].mask.sub<T>();
}

template <uint32_t typeWidth>
template <typename ...Ts>
bool EntityManager<typeWidth>::has(uint64_t id) const {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling has");
		return false;
	}

	if (!hasFlags(_identities[index].flags, Identity::Active))
		return false;

	return _identities[index].mask.has<Ts...>();
}

template<uint32_t typeWidth>
template <typename ...Ts>
void EntityManager<typeWidth>::reserve(uint32_t count) {
	_reserve<0, Ts...>(count - 1);
}

template<uint32_t typeWidth>
void EntityManager<typeWidth>::clear() {
	for (uint32_t i = 0; i < _identities.size(); i++) {
		if (_identities[i].mask.empty() || !hasFlags(_identities[i].flags, Identity::Active))
			continue;

		if (_identities[i].references)
			_warning(Warning::References, std::string("calling clear with references still existing") );

		_erase(i);
	}
}

template <uint32_t typeWidth>
uint32_t EntityManager<typeWidth>::count() const {
	return (_identities.size() + _buffered.size()) - _freeIndexes.size();
}

template <uint32_t typeWidth>
template <typename ...Ts, typename T>
void EntityManager<typeWidth>::iterate(const T& lambda) {
	std::tuple<Ts*...> components;

	// if already iterating (in case of nested iterate)
	bool iterating = _iterating;

	if (!iterating)
		_iterating = true;

	// main iteration
	for (const Identity& i : _identities)
		_iterate<Ts...>(i, lambda);

	// iterate over buffered
	while (_buffered.size()) {
		Identity& identity = _identities[_buffered.front()];		

		_iterate<Ts...>(identity, lambda);

		identity.flags &= ~Identity::Buffered;
		_buffered.erase(_buffered.begin());
	}

	if (!iterating)
		_iterating = false;
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::reference(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling reference");
		return;
	}

	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity
	
	_identities[index].references++;
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::dereference(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling dereference");
		return;
	}

	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity;

	if (!_identities[index].references) {
		_warning(Warning::References, "calling dereference with no more references");
		return;
	}

	_identities[index].references--;

	if (_identities[index].references == 0 && hasFlags(_identities[index].flags, Identity::Erased))
		_erase(index);
}

template <uint32_t typeWidth>
void EntityManager<typeWidth>::setEnabled(uint64_t id, bool enabled) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version)) {
		_warning(Warning::Invalid, "calling setEnabled");
		return;
	}

	assert(hasFlags(_identities[index].flags, Identity::Active)); // sanity;

	if (!enabled)
		_identities[index].flags &= ~Identity::Enabled;
	else
		_identities[index].flags |= Identity::Enabled;
}