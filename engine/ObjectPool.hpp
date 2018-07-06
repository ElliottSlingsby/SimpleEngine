#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <utility>
#include <vector>

class BasePool {
protected:
	const size_t _chunkSize;
	const size_t _elementSize;

	std::vector<uint8_t*> _chunks;

public:
	inline BasePool(size_t elementSize, size_t chunkSize);

	inline ~BasePool();

	inline void reserve(uint32_t index);

	template <typename T>
	inline T* get(uint32_t index);

	template <typename T, typename ...Ts>
	inline void insert(uint32_t index, Ts&&... args);

	inline uint32_t count() const;

	virtual inline void erase(uint32_t index) = 0;
};

template <typename T>
class ObjectPool : public BasePool {
	template <typename T1>
	inline void _erase(uint32_t index);

public:
	inline ObjectPool(size_t chunkSize);

	inline void erase(uint32_t index) override;
};

BasePool::BasePool(size_t elementSize, size_t chunkSize) : _elementSize(elementSize), _chunkSize(chunkSize) {}

BasePool::~BasePool() {
	for (uint8_t* chunk : _chunks)
		free(chunk);
}

void BasePool::reserve(uint32_t index) {
	if (index < count())
		return;

	size_t elementsPerChunk = _chunkSize / _elementSize;

	assert(index / elementsPerChunk <= UINT32_MAX);
	uint32_t chunk = static_cast<uint32_t>(index / elementsPerChunk);

	size_t size = _chunks.size();
	_chunks.resize(chunk + 1);

	for (size_t i = size; i < chunk + 1; i++) {
		_chunks[i] = static_cast<uint8_t*>(malloc(_chunkSize));
		assert(_chunks[i]);
	}
}

template <typename T>
T* BasePool::get(uint32_t index) {
	assert(index < count());

	size_t elementsPerChunk = _chunkSize / _elementSize;

	assert(index / elementsPerChunk <= UINT32_MAX);
	uint32_t chunk = static_cast<uint32_t>(index / elementsPerChunk);

	size_t offset = (index - (chunk * elementsPerChunk)) * _elementSize;

	return reinterpret_cast<T*>(_chunks[chunk] + offset);
}

template <typename T, typename ...Ts>
void BasePool::insert(uint32_t index, Ts&&... args) {
	assert(sizeof(T) <= _elementSize);

	if (index >= count())
		reserve(index);

	new(static_cast<void*>(get<T>(index))) T(std::forward<Ts>(args)...);
}

uint32_t BasePool::count() const {
	size_t elementsPerChunk = _chunkSize / _elementSize;
	assert(_chunks.size() * elementsPerChunk <= UINT32_MAX);
	return static_cast<uint32_t>(_chunks.size() * elementsPerChunk);
}

template<typename T>
ObjectPool<T>::ObjectPool(size_t chunkSize) : BasePool(sizeof(T), chunkSize) { }

template<typename T>
template<typename T1>
void ObjectPool<T>::_erase(uint32_t index) {
	get<T>(index)->~T();
}

template <typename T>
void ObjectPool<T>::erase(uint32_t index) {
	_erase<T>(index);
}