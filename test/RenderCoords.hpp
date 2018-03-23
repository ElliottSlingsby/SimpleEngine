#pragma once

#include <glm/vec3.hpp>

template <typename T>
struct GlobalTVec3 {
	static const glm::tvec3<T> up;
	static const glm::tvec3<T> down;
	static const glm::tvec3<T> left;
	static const glm::tvec3<T> right;
	static const glm::tvec3<T> forward;
	static const glm::tvec3<T> back;
};

template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::up = { 0, 0, 1 };
template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::down = { 0, 0, -1 };
template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::left = { -1, 0, 0 };
template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::right = { 1, 0, 0 };
template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::forward = { 0, 1, 0 };
template <typename T>
const glm::tvec3<T> GlobalTVec3<T>::back = { 0, -1, 0 };

template <typename T>
struct LocalTVec3 {
	static const glm::tvec3<T> up;
	static const glm::tvec3<T> down;
	static const glm::tvec3<T> left;
	static const glm::tvec3<T> right;
	static const glm::tvec3<T> forward;
	static const glm::tvec3<T> back;
};

template <typename T>
const glm::tvec3<T> LocalTVec3<T>::up = { 0, 1, 0 };
template <typename T>
const glm::tvec3<T> LocalTVec3<T>::down = { 0, -1, 0 };
template <typename T>
const glm::tvec3<T> LocalTVec3<T>::left = { -1, 0, 0 };
template <typename T>
const glm::tvec3<T> LocalTVec3<T>::right = { 1, 0, 0 };
template <typename T>
const glm::tvec3<T> LocalTVec3<T>::forward = { 0, 0, -1 };
template <typename T>
const glm::tvec3<T> LocalTVec3<T>::back = { 0, 0, 1 };

using GlobalVec3 = GlobalTVec3<float>;
using LocalVec3 = LocalTVec3<float>;

using GlobalDVec3 = GlobalTVec3<double>;
using LocalDVec3 = LocalTVec3<double>;

using GlobalIVec3 = GlobalTVec3<int>;
using LocalDIec3 = LocalTVec3<int>;