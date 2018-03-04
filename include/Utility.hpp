#pragma once

#include <cstdint>
#include <chrono>
#include <thread>
#include <utility>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

struct BasePtr {
	void* const ptr;

	inline BasePtr(void* pointer) : ptr(pointer) {}

	virtual inline ~BasePtr() {}
};

template <typename T>
struct VirtualPtr : public BasePtr {
	template <typename ...Ts>
	inline VirtualPtr(Ts&&... args) : BasePtr(new T(std::forward<Ts>(args)...)){}

	inline ~VirtualPtr() {
		delete static_cast<T*>(ptr);
	}
};

inline uint32_t back64(uint64_t i) {
	return static_cast<uint32_t>(i);
}

inline uint32_t front64(uint64_t i) {
	return static_cast<uint32_t>(i >> 32);
}

inline uint64_t combine32(uint32_t front, uint32_t back) {
	return static_cast<uint64_t>(back) + (static_cast<uint64_t>(front) << 32);
}

inline bool hasFlags(uint8_t target, uint8_t check) {
	return (target & check) == check;
}

inline void startTime(TimePoint* point) {
	*point = Clock::now();
}

template <typename T = double>
inline T deltaTime(const TimePoint& point) {
	return std::chrono::duration_cast<std::chrono::duration<T>>(Clock::now() - point).count();
}

template <typename T = double>
inline void sleepFor(T seconds) {
	if (seconds < 0)
		return;

	std::this_thread::sleep_for(std::chrono::microseconds(static_cast<uint32_t>(seconds * 1000000)));
}

inline void clearTerminal() {
#ifdef _WIN32
	std::system("cls");
#endif
#ifdef __linux__
	std::system("clear");
#endif
}

template <typename Namespace>
inline uint32_t typeIndexCount(bool add = false) {
	static uint32_t count = 0;

	if (add)
		return count++;

	return count;
}

template <typename Namespace, typename T>
inline uint32_t typeIndex() {
	static uint32_t index = typeIndexCount<Namespace>(true);
	return index;
}