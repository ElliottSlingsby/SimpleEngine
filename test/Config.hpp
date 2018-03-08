#pragma once

#include <SimpleEngine.hpp>

#define MAX_COMPONENTS 16
#define MAX_EVENTS 32
#define MAX_LISTENERS 8

using Engine = SimpleEngine<MAX_COMPONENTS, MAX_EVENTS, MAX_LISTENERS>;

enum Events {
	Load,
	Update,

};

const size_t chunkSize = 1024 * 1024 * 128; // 128 mb