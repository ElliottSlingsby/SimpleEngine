#pragma once

#include "TypeMask.hpp"
#include "ObjectPool.hpp"
#include "Utility.hpp"

#include <vector>
#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <iostream>

#define SYSFUNC(systemInterface, systemFunction) \
	systemInterface::Engine::BaseSystem::FunctionSpecialization<decltype(&systemInterface::systemFunction), &systemInterface::systemFunction>

#define SYSFUNC_ENABLE(systemInterface, systemFunction, priority) \
	SYSFUNC(systemInterface, systemFunction)::enable<std::remove_reference<decltype(*this)>::type>(priority)

#define SYSFUNC_CALL(systemInterface, systemFunction, engine) \
	engine.callSystems<SYSFUNC(systemInterface, systemFunction)>

template <typename SystemInterface, uint32_t maxComponents>
class SimpleEngine {
	using TypeMask = TypeMask<maxComponents>;

	struct Identity{ 
		uint32_t index;
		uint32_t version = 1;
		TypeMask mask;
		uint8_t flags = None;
		uint32_t references = 0;

		enum Flags : uint8_t{
			None = 0,
			Active = 1,
			Destroyed = 2,
			Buffered = 4
		};
	};

	struct IndexPriorityPair {
		uint32_t index;
		int32_t priority = 0;

		inline void operator=(const IndexPriorityPair& other) {
			index = other.index;
			priority = other.priority;
		}

		inline bool operator<(const IndexPriorityPair& other) {
			return priority < other.priority;
		}

		inline bool operator==(const IndexPriorityPair& other) {
			return index == other.index;
		}
	};

public:
	class BaseSystem {
	public:
		using Engine = SimpleEngine;

		template <typename T, T> struct FunctionSpecialization;

		template <typename T, typename ...Ts, void(T::*func)(Ts...)>
		class FunctionSpecialization<void(T::*)(Ts...), func> {
			static std::vector<IndexPriorityPair> _systemIndexes;

		public:
			typedef void(T::*FuncType)(Ts...);

			static const FuncType functionPtr;

			template <typename SystemT>
			inline static void enable(int32_t priority = 0);

			inline static uint32_t systemCount();

			inline static uint32_t systemIndex(uint32_t i);
		};

		inline virtual ~BaseSystem() {}
	};

	class Entity {
		SimpleEngine& _engine;
		uint64_t _id = 0;

	public:
		inline Entity(SimpleEngine& engine) : _engine(engine) {}

		inline uint64_t id() const;
		
		inline void create();

		inline void destroy();

		inline bool valid() const;

		inline void invalidate();
		
		template <typename T, typename ...Ts>
		inline T* add(Ts&&... args);

		template <typename T>
		inline T* get();

		template <typename T>
		inline void remove();

		template <typename ...Ts>
		inline bool has() const;

		inline void set(uint64_t id);
	};

private:
	size_t _chunkSize;
	
	bool _running = true;

	std::vector<SystemInterface*> _systems;

	std::vector<Identity> _indexIdentities;
	std::vector<uint32_t> _freeIndexes;

	BasePool* _componentPools[maxComponents] = { nullptr };

	std::vector<uint32_t> _bufferedIndexes;
	bool _iterating = false;

	inline bool _validId(uint32_t index, uint32_t version) const;

	inline void _destroy(uint32_t index);

	template <typename T>
	inline void _iterate(uint32_t index, const T& lambda);

public:
	SimpleEngine(size_t chunkSize) : _chunkSize(chunkSize) {}

	template <typename T, typename ...Ts>
	inline void registerSystem(Ts&&... args);

	template <typename T>
	inline bool hasSystem();

	template <typename T>
	inline T& system();

	template <typename T, typename ...Ts>
	inline void callSystems(Ts&&... args);

	inline bool running() const;

	inline void quit();

	inline uint64_t createEntity();

	inline bool validEntity(uint64_t id);

	inline void destroyEntity(uint64_t id);

	template <typename T, typename ...Ts>
	inline T* addComponent(uint64_t id, Ts&&... args);

	template <typename T>
	inline T* getComponent(uint64_t id);

	template <typename T>
	inline void removeComponent(uint64_t id);

	template <typename ...Ts>
	inline bool hasComponents(uint64_t id) const;

	inline void referenceEntity(uint64_t id);

	inline void dereferenceEntity(uint64_t id);

	inline uint32_t entityCount() const;
	
	template <typename T>
	inline void iterateEntities(const T& lambda);
};

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
const typename SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::FuncType SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::functionPtr = func;

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
std::vector<typename SimpleEngine<SystemInterface, maxComponents>::IndexPriorityPair> SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::_systemIndexes;

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
template <typename SystemT>
void SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::enable(int32_t priority) {
	static_assert(std::is_base_of<BaseSystem, SystemInterface>::value);

	IndexPriorityPair indexPriority = { typeIndex<SimpleEngine, SystemT>(), priority };

	auto iter = std::find(_systemIndexes.begin(), _systemIndexes.end(), indexPriority);
	
	if (iter != _systemIndexes.end())
		iter->priority = priority;
	else
		_systemIndexes.push_back(indexPriority);

	std::sort(_systemIndexes.begin(), _systemIndexes.end());
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
uint32_t SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::systemCount(){
	assert(_systemIndexes.size() <= UINT32_MAX);
	return static_cast<uint32_t>(_systemIndexes.size());
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
uint32_t SimpleEngine<SystemInterface, maxComponents>::BaseSystem::FunctionSpecialization<void(T::*)(Ts...), func>::systemIndex(uint32_t i){
	assert(i < _systemIndexes.size());
	return _systemIndexes[i].index;
}

template <typename SystemInterface, uint32_t maxComponents>
bool SimpleEngine<SystemInterface, maxComponents>::_validId(uint32_t index, uint32_t version) const {
	if (index >= _indexIdentities.size())
		return false;

	return version == _indexIdentities[index].version;
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::_destroy(uint32_t index) {
	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity

	// if references still exist, mark as erased for later, and return
	if (_indexIdentities[index].references) {
		_indexIdentities[index].flags |= Identity::Destroyed;
		return;
	}

	// remove maxComponents from each pool
	for (uint32_t i = 0; i < maxComponents; i++) {
		if (_indexIdentities[index].mask.has(i)) {
			assert(_componentPools[i]); // sanity
			_componentPools[i]->erase(index);
			_indexIdentities[index].mask.sub(i);
		}
	}

	// clear up identity, increment version
	_indexIdentities[index].version++;
	_indexIdentities[index].flags = Identity::None;

	_freeIndexes.push_back(index);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
void SimpleEngine<SystemInterface, maxComponents>::_iterate(uint32_t index, const T& lambda) {
	Identity& identity = _indexIdentities[index];

	if (!hasFlags(identity.flags, Identity::Active) || hasFlags(identity.flags, Identity::Buffered) || hasFlags(identity.flags, Identity::Destroyed))
		return;

	Entity entity(*this);
	entity.set(combine32(identity.index, identity.version));

	lambda(entity);
}

template <typename SystemInterface, uint32_t maxComponents>
template<typename T, typename ...Ts>
void SimpleEngine<SystemInterface, maxComponents>::registerSystem(Ts&&... args){
	static_assert(std::is_base_of<BaseSystem, SystemInterface>::value);
	static_assert(std::is_base_of<SystemInterface, T>::value);

	uint32_t index = typeIndex<SimpleEngine, T>();

	if (_systems.size() <= index)
		_systems.resize(index + 1);

	assert(_systems[index] == nullptr);

	_systems[index] = new T(std::forward<Ts>(args)...);
}

template<typename SystemInterface, uint32_t maxComponents>
template<typename T>
bool SimpleEngine<SystemInterface, maxComponents>::hasSystem(){
	uint32_t index = typeIndex<SimpleEngine, T>();

	if (_systems.size() <= index)
		return false;

	return _systems[index] != nullptr;
}

template <typename SystemInterface, uint32_t maxComponents>
template<typename T>
T& SimpleEngine<SystemInterface, maxComponents>::system(){
	uint32_t index = typeIndex<SimpleEngine, T>();

	if (_systems.size() <= index)
		_systems.resize(index + 1);

	assert(_systems[index] != nullptr);

	return *static_cast<T*>(_systems[index]);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts>
void SimpleEngine<SystemInterface, maxComponents>::callSystems(Ts&&... args) {
	for (uint32_t i = 0; i < T::systemCount(); i++) {
		(_systems[T::systemIndex(i)]->*T::functionPtr)(std::forward<Ts>(args)...);
	}
}

template <typename SystemInterface, uint32_t maxComponents>
bool SimpleEngine<SystemInterface, maxComponents>::running() const {
	return _running;
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::quit(){
	_running = false;
}

template <typename SystemInterface, uint32_t maxComponents>
uint64_t SimpleEngine<SystemInterface, maxComponents>::createEntity() {
	// find free index
	uint32_t index;

	if (_freeIndexes.empty()) {
		assert(_indexIdentities.size() <= UINT32_MAX);
		index = static_cast<uint32_t>(_indexIdentities.size());
		_indexIdentities.push_back({ index, 1 });
	}
	else {
		index = *_freeIndexes.rbegin();
		_freeIndexes.pop_back();
	}

	// set identity flags to active
	assert(!_indexIdentities[index].references); // sanity
	assert(!hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity
	_indexIdentities[index].flags = Identity::Active;

	// if made during iteration, buffer for later
	if (_iterating) {
		_indexIdentities[index].flags |= Identity::Buffered;
		_bufferedIndexes.push_back(index);
	}

	// return index and version combined
	return combine32(index, _indexIdentities[index].version);
}

template <typename SystemInterface, uint32_t maxComponents>
bool SimpleEngine<SystemInterface, maxComponents>::validEntity(uint64_t id) {
	if (id == 0)
		return false;

	uint32_t index = front64(id);
	uint32_t version = back64(id);

	return _validId(index, version);
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::destroyEntity(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	assert(_validId(index, version) && "calling erase with invalid id");

	if (!_validId(index, version))
		return;

	_destroy(index);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts>
T* SimpleEngine<SystemInterface, maxComponents>::addComponent(uint64_t id, Ts&&... args) {
	static_assert(std::is_constructible<T, Ts...>::value);

	uint32_t index = front64(id);
	uint32_t version = back64(id);

	assert(_validId(index, version) && "calling add with invalid id");

	if (!_validId(index, version))
		return nullptr;

	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity

	uint32_t type = TypeMask::index<T>();

	if (_indexIdentities[index].mask.has<T>())
		return _componentPools[TypeMask::index<T>()]->get<T>(index);

	// update identity
	_indexIdentities[index].mask.add<T>();

	// create pool if it doesn't exist
	if (_componentPools[type] == nullptr)
		_componentPools[type] = new ObjectPool<T>(_chunkSize);

	_componentPools[TypeMask::index<T>()]->insert<T>(index, std::forward<Ts>(args)...);

	return _componentPools[TypeMask::index<T>()]->get<T>(index);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
T* SimpleEngine<SystemInterface, maxComponents>::getComponent(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version))
		return false;

	uint32_t type = TypeMask::index<T>();

	if (_componentPools[type] == nullptr)
		return nullptr;

	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity

	if (!_indexIdentities[index].mask.has<T>())
		return nullptr;

	return _componentPools[TypeMask::index<T>()]->get<T>(index);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
void SimpleEngine<SystemInterface, maxComponents>::removeComponent(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	if (!_validId(index, version))
		return;

	uint32_t type = TypeMask::index<T>();

	assert(_componentPools[type] != nullptr); // sanity
	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity

	if (!_indexIdentities[index].mask.has<T>())
		return;

	// remove from pool
	_componentPools[type]->erase(index);

	// update identity
	_indexIdentities[index].mask.sub<T>();
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename ...Ts>
bool SimpleEngine<SystemInterface, maxComponents>::hasComponents(uint64_t id) const {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	assert(_validId(index, version) && "calling has with invalid id");

	if (!_validId(index, version))
		return false;

	if (!hasFlags(_indexIdentities[index].flags, Identity::Active))
		return false;

	return _indexIdentities[index].mask.has<Ts...>();
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::referenceEntity(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	assert(_validId(index, version) && "calling reference with invalid id");

	if (!_validId(index, version))
		return;

	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity

	_indexIdentities[index].references++;
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::dereferenceEntity(uint64_t id) {
	uint32_t index = front64(id);
	uint32_t version = back64(id);

	assert(_validId(index, version) && "calling dereference with invalid id");

	if (!_validId(index, version))
		return;

	assert(hasFlags(_indexIdentities[index].flags, Identity::Active)); // sanity;

	assert(_indexIdentities[index].references && "calling dereference with no more references");

	if (!_indexIdentities[index].references)
		return;

	_indexIdentities[index].references--;

	if (_indexIdentities[index].references == 0 && hasFlags(_indexIdentities[index].flags, Identity::Destroyed))
		_destroy(index);
}

template <typename SystemInterface, uint32_t maxComponents>
uint32_t SimpleEngine<SystemInterface, maxComponents>::entityCount() const {
	return (_indexIdentities.size() + _bufferedIndexes.size()) - _freeIndexes.size();
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
void SimpleEngine<SystemInterface, maxComponents>::iterateEntities(const T& lambda) {
	_iterating = true;

	for (uint32_t i = 0; i < _indexIdentities.size(); i++) 
		_iterate(i, lambda);

	while (_bufferedIndexes.size()) {
		_iterate(_bufferedIndexes[0], lambda);
		_bufferedIndexes.erase(_bufferedIndexes.begin());
	}

	_iterating = false;
}

template <typename SystemInterface, uint32_t maxComponents>
uint64_t SimpleEngine<SystemInterface, maxComponents>::Entity::id() const {
	return _id;
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::Entity::create() {
	if (_id)
		invalidate();

	_id = _engine.createEntity();
	_engine.referenceEntity(_id);
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::Entity::destroy() {
	assert(_id);

	if (!_id)
		return;

	_engine.dereferenceEntity(_id);
	_engine.destroyEntity(_id);
	_id = 0;
}

template <typename SystemInterface, uint32_t maxComponents>
bool SimpleEngine<SystemInterface, maxComponents>::Entity::valid() const {
	return _id;
}

template <typename SystemInterface, uint32_t maxComponents>
void SimpleEngine<SystemInterface, maxComponents>::Entity::invalidate() {
	assert(_id);

	if (!_id)
		return;

	_engine.dereferenceEntity(_id);
	_id = 0;
}

template<typename SystemInterface, uint32_t maxComponents>
inline void SimpleEngine<SystemInterface, maxComponents>::Entity::set(uint64_t id){
	if (_id)
		invalidate();

	if (!_engine.validEntity(id))
		return;
	
	_id = id;
	_engine.referenceEntity(_id);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T, typename ...Ts>
T* SimpleEngine<SystemInterface, maxComponents>::Entity::add(Ts&&... args) {
	assert(_id);

	if (!_id)
		return nullptr;

	return _engine.addComponent<T>(_id, std::forward<Ts>(args)...);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
T* SimpleEngine<SystemInterface, maxComponents>::Entity::get() {
	assert(_id);

	if (!_id)
		return nullptr;

	return _engine.getComponent<T>(_id);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename T>
void SimpleEngine<SystemInterface, maxComponents>::Entity::remove() {
	assert(_id);

	if (!_id)
		return;

	_engine.removeComponent<T>(_id);
}

template <typename SystemInterface, uint32_t maxComponents>
template <typename ...Ts>
bool SimpleEngine<SystemInterface, maxComponents>::Entity::has() const {
	assert(_id);

	if (!_id)
		return false;

	return _engine.hasComponents<Ts...>(_id);
}