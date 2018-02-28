#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>

class BasePool {
protected:
	const size_t _chunkSize;
	const size_t _objectSize;

	uint8_t* _buffer = nullptr;
	size_t _size = 0;

	inline uint8_t* _conv(void* memory) const;

public:
	inline BasePool(size_t elementSize, size_t chunkSize);

	inline ~BasePool();

	inline void reserve(uint32_t index);

	template <typename T>
	inline T* get(uint32_t index);

	template <typename T, typename ...Ts>
	inline void insert(uint32_t index, Ts... args);

	inline size_t size() const;

	inline uint32_t count() const;

	virtual inline void erase(uint32_t index) = 0;
};

template <class T>
class ObjectPool : public BasePool {
public:
	ObjectPool(size_t chunkSize);

	inline void erase(uint32_t index) override;
};

BasePool::BasePool(size_t elementSize, size_t chunkSize) : _objectSize(elementSize), _chunkSize(chunkSize) {
	reserve(0);
}

BasePool::~BasePool() {
	free(_buffer);
}

void BasePool::reserve(uint32_t index) {
	if (index > count())
		return;

	_size = (index * _objectSize) + _chunkSize - ((index * _objectSize) % _chunkSize);

	if (_buffer == nullptr)
		_buffer = _conv(malloc(_size));
	else
		_buffer = _conv(realloc(_buffer, _size));
}

uint8_t* BasePool::_conv(void* memory) const {
	assert(memory != nullptr);

	return static_cast<uint8_t*>(memory);
}

template<typename T>
T* BasePool::get(uint32_t index) {
	assert(sizeof(T) <= _objectSize);
	assert(index < count());

	return reinterpret_cast<T*>(_buffer + (index * _objectSize));
}

template <typename T, typename ...Ts>
void BasePool::insert(uint32_t index, Ts... args) {
	assert(sizeof(T) <= _objectSize);

	if (index >= count())
		reserve(index);

	new(static_cast<void*>(_buffer + (index * _objectSize))) T(args...);
}

size_t BasePool::size() const {
	return _size;
}

uint32_t BasePool::count() const {
	return static_cast<uint32_t>(_size / _objectSize);
}

template<class T>
ObjectPool<T>::ObjectPool(size_t chunkSize) : BasePool(sizeof(T), chunkSize) { }

template<class T>
void ObjectPool<T>::erase(uint32_t index) {
	get<T>(index)->~T();
}