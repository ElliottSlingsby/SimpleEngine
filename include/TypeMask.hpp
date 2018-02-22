#pragma once

#include <cstdint>
#include <bitset>
#include <type_traits>
#include <cassert>

template <size_t width>
class TypeMask {
	std::bitset<width> _mask;
	static uint32_t _typeCount;

	template <uint32_t i, typename ...T>
	inline typename std::enable_if<i == sizeof...(T), void>::type _fill(bool value);

	template <uint32_t i, typename ...T>
	inline typename std::enable_if < i < sizeof...(T), void>::type _fill(bool value);

public:
	template <typename T>
	inline static uint32_t index();

	inline TypeMask<width>& operator=(const TypeMask<width>& other);

	template <typename ...T>
	inline void fill();

	template <typename ...T>
	inline void add();

	template <typename ...T>
	inline void sub();

	template <typename ...T>
	inline bool has() const;

	inline bool has(uint32_t i) const;

	inline bool empty() const;

	inline void clear();

	template <typename ...T>
	inline static TypeMask<width> create();
};

template<size_t width>
uint32_t TypeMask<width>::_typeCount = 0;

template <size_t width>
template <uint32_t i, typename ...T>
typename std::enable_if<i == sizeof...(T), void>::type TypeMask<width>::_fill(bool value) { }

template <size_t width>
template <uint32_t i, typename ...T>
typename std::enable_if<i < sizeof...(T), void>::type TypeMask<width>::_fill(bool value) {
	_fill<i + 1, T...>(value);

	using Type = std::tuple_element<i, std::tuple<T...>>::type;
	_mask.set(index<Type>(), value);
}

template <size_t width>
template <typename T>
uint32_t TypeMask<width>::index() {
	static const uint32_t index = _typeCount++;

	assert(index < width && "too many types, increase width");
	
	return index;
}

template <size_t width>
TypeMask<width>& TypeMask<width>::operator=(const TypeMask<width>& other) {
	_mask = other._mask;
	return *this;
}

template <size_t width>
template <typename ...T>
void TypeMask<width>::fill() {
	_mask.reset();
	_fill<0, T...>(true);
}

template <size_t width>
template <typename ...T>
void TypeMask<width>::add() {
	_fill<0, T...>(true);
}

template <size_t width>
template <typename ...T>
void TypeMask<width>::sub() {
	_fill<0, T...>(false);
}

template <size_t width>
template <typename ...T>
bool TypeMask<width>::has() const {
	TypeMask<width> other = create<T...>();

	unsigned long check = other._mask.to_ulong();
	return (_mask.to_ulong() & check) == check;
}

template<size_t width>
bool TypeMask<width>::has(uint32_t i) const {
	return _mask[i];
}

template <size_t width>
bool TypeMask<width>::empty() const {
	return _mask.to_ulong() == 0;
}

template<size_t width>
void TypeMask<width>::clear() {
	_mask = 0;
}

template <size_t width>
template <typename ...T>
TypeMask<width> TypeMask<width>::create() {
	TypeMask<width> mask;
	mask.fill<T...>();

	return mask;
}