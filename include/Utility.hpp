#pragma once

#include <cstdint>

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