#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

class BasePool {
protected:
	const size_t _chunkSize;
	const size_t _objectSize;

	uint8_t* _buffer = nullptr;
	size_t _size = 0;

	inline uint8_t* _conv(void* memory) const;

	inline void _extend(uint32_t index);

public:
	inline BasePool(size_t elementSize, size_t chunkSize);

	inline ~BasePool();

	template <typename T>
	inline T* get(uint32_t index);

	template <typename T, typename ...Ts>
	inline void insert(uint32_t index, Ts... args);

	inline size_t size() const;

	inline uint32_t count() const;

	virtual inline void erase(uint32_t index) = 0;

	virtual inline void print() const = 0;
};

template <class T>
class ObjectPool : public BasePool {
public:
	ObjectPool(size_t chunkSize);

	inline void print() const override;

	inline void erase(uint32_t index) override;
};

BasePool::BasePool(size_t elementSize, size_t chunkSize) : _objectSize(elementSize), _chunkSize(chunkSize) {
	_extend(0);
}

BasePool::~BasePool() {
	free(_buffer);
}

uint8_t* BasePool::_conv(void* memory) const {
	assert(memory != nullptr);

	return static_cast<uint8_t*>(memory);
}

void BasePool::_extend(uint32_t index) {
	assert(index >= count());

	_size = (index * _objectSize) + _chunkSize - ((index * _objectSize) % _chunkSize);

	if (_buffer == nullptr)
		_buffer = _conv(malloc(_size));
	else
		_buffer = _conv(realloc(_buffer, _size));
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
		_extend(index);

	new(static_cast<void*>(_buffer + (index * _objectSize))) T(args...);
}

size_t BasePool::size() const {
	return _size;
}

uint32_t BasePool::count() const {
	return static_cast<uint32_t>(_size / _objectSize);
}

template<class T>
ObjectPool<T>::ObjectPool(size_t chunkSize) : BasePool(sizeof(T), chunkSize) {}

template<class T>
void ObjectPool<T>::print() const {
#ifdef _DEBUG
	for (size_t i = 0; i < _size; i++) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(_buffer[i]);
	
		if (i != _size - 1 && i % sizeof(T) != sizeof(T) - 1)
			std::cout << ' ';
		else
			std::cout << std::endl;
	}
#endif
}

template<class T>
void ObjectPool<T>::erase(uint32_t index) {
	get<T>(index)->~T();
}