#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <set>
#include <optional>
#include <cassert>

namespace SimpleEngine{

template <uint32_t events, uint32_t listeners>
class EventHandler {
	struct BaseFunction {
		void* const ptr;

		inline BaseFunction(void* func) : ptr(func) {}

		inline virtual ~BaseFunction() {}

		template <typename ...Ts>
		inline void call(Ts... args) {
			(*static_cast<std::function<void(Ts...)>*>(ptr))(args...);
		}
	};

	template <typename T>
	struct Function : public BaseFunction {
		inline Function(const T& func) : BaseFunction(new T(func)) {}

		inline ~Function() {
			delete static_cast<T*>(ptr);
		}
	};

	struct EventLink {
		uint32_t index;
		uint32_t listener;
	};

	std::optional<BaseFunction> _functions[events * listeners] = { }; // Array of BaseFunctions (16 events * 16 listeners per event)
	void* _objects[events * listeners] = { nullptr }; // ptrs for getting data from _eventLinks map

	uint32_t _functionIndexes = 0;
	std::vector<uint32_t> _freeIndexes;

	uint32_t _listenerCount[events] = { 0 };
	uint32_t _eventListeners[events][listeners] = { { 0 } }; // 2D array linking events to listeners functions in _listenersX (16 events, 16 listeners per event)

	std::unordered_map<void*, std::unordered_map<uint32_t, EventLink>> _eventLinks; // map for unsubscribing

	template <class ...Ts, typename T, int... indices>
	inline std::function<void(Ts...)> _bind(void (T::*func)(Ts...), T* obj, std::integer_sequence<int, indices...>);

	template <class ...Ts, typename T>
	inline std::function<void(Ts...)> _bind(void (T::*func)(Ts...), T* obj);

public:
	template <typename T, typename ...Ts>
	inline void subscribe(T* obj, int event, void (T::*func)(Ts...));

	template <typename T>
	inline void unsubscribe(T* obj, int event);

	template <typename T>
	inline void unsubscribe(T* obj);

	inline void unsubscribe(int event);

	template <typename ...Ts>
	inline void dispatch(int event, Ts... args);
};

template <uint32_t events, uint32_t listeners>
template <class ...Ts, typename T, int... indices>
std::function<void(Ts...)> EventHandler<events, listeners>::_bind(void (T::*func)(Ts...), T* obj, std::integer_sequence<int, indices...>) {
	return std::bind(func, obj, std::_Ph<indices + 1>()...);
}

template <uint32_t events, uint32_t listeners>
template <class ...Ts, typename T>
std::function<void(Ts...)> EventHandler<events, listeners>::_bind(void (T::*func)(Ts...), T* obj) {
	return _bind(func, obj, std::make_integer_sequence<int, sizeof...(Ts)>());
}

template <uint32_t events, uint32_t listeners>
template <typename T, typename ...Ts>
void EventHandler<events, listeners>::subscribe(T* obj, int event, void (T::*func)(Ts...)) {
	if (_eventLinks[obj].find(event) != _eventLinks[obj].end()) // check
		unsubscribe(obj, event);

	uint32_t index;

	if (_freeIndexes.size()) {
		index = *_freeIndexes.rbegin();
		_freeIndexes.pop_back();
	}
	else {
		index = _functionIndexes;
		_functionIndexes++;

		assert(_functionIndexes < events * listeners && "no more function slots");
	}

	assert(event < events && "no more event slots");
	assert(_listenerCount[event] < listeners && "no more listener slots");

	_eventLinks[obj][event] = { index, _listenerCount[event] };

	new (&_functions[index]) std::optional<Function<std::function<void(Ts...)>>>(_bind(func, obj));
	_objects[index] = obj;

	_eventListeners[event][_listenerCount[event]] = index;
	_listenerCount[event]++;
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj, int event) {
	assert(event < events && "invalid event slot");

	if (_eventLinks[obj].find(event) == _eventLinks[obj].end()) // check
		return;

	assert(_listenerCount[event]); // sanity
	assert(_functions[_eventLinks[obj][event].index].has_value()); // sanity

	uint32_t listener = _eventLinks[obj][event].listener;
	uint32_t index = _eventLinks[obj][event].index;

	_functions[index]->~BaseFunction();
	_functions[index] = std::nullopt;
	_freeIndexes.push_back(index);

	_eventLinks[obj].erase(event);

	uint32_t lastListener = _listenerCount[event] - 1;
	uint32_t lastIndex = _eventListeners[event][lastListener];

	_eventListeners[event][listener] = lastIndex;
	_listenerCount[event]--;
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj) {
	if (_eventLinks[obj].size() == 0) // check
		return;

	std::vector<uint32_t> events;
	events.reserve(_eventLinks[obj].size());

	for (const auto& i : _eventLinks[obj])
		events.push_back(i.first);

	for (uint32_t i : events) 
		unsubscribe(obj, i);
}

template<uint32_t events, uint32_t listeners>
void EventHandler<events, listeners>::unsubscribe(int event){
	if (_listenerCount[event] == 0) // check
		return;

	std::vector<uint32_t> indexes;
	indexes.reserve(_listenerCount[event]);

	for (uint32_t i = 0; i < _listenerCount[event]; i++)
		indexes.push_back(_eventListeners[event][i]);

	for (uint32_t i : indexes)
		unsubscribe(_objects[i], event);
}

template <uint32_t events, uint32_t listeners>
template <typename ...Ts>
void EventHandler<events, listeners>::dispatch(int event, Ts... args) {
	assert(event < events && "invalid event slot");

	if (_listenerCount[event] == 0) // check
		return;

	std::vector<uint32_t> indexes;
	indexes.reserve(_listenerCount[event]);

	for (uint32_t i = 0; i < _listenerCount[event]; i++)
		indexes.push_back(_eventListeners[event][i]);

	for (uint32_t i : indexes)
		_functions[i]->call(args...);
}

}