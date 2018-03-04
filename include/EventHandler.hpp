#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <set>
#include <optional>
#include <cassert>

template <uint32_t events, uint32_t listeners>
class EventHandler {
	// virtually destructed object for storing binded object and member function with generic arguments
	struct BaseFunction {
		void* const ptr;

		inline BaseFunction(void* func) : ptr(func) {}

		inline virtual ~BaseFunction() {}

		template <typename ...Ts>
		inline void call(Ts... args) {
			(*static_cast<std::function<void(Ts...)>*>(ptr))(args...);
		}
	};
	
	// object for creating and deleting dynamic memory containing specialized function pointer
	template <typename T>
	struct Function : public BaseFunction {
		inline Function(const T& func) : BaseFunction(new T(func)) {}

		inline ~Function() {
			delete static_cast<T*>(ptr);
		}
	};

	// stores information about a subscription
	struct Listener {
		std::optional<BaseFunction> function;
		void* object = nullptr;
		int32_t order = 0;
	};

	// stores a back-reference to an event
	struct EventLink {
		uint32_t index;
		uint32_t listener;
	};

	// listener objects for each subscription
	Listener _listenerData[events * listeners] = { };

	uint32_t _functionIndexes = 0;
	std::vector<uint32_t> _freeIndexes;

	// 2D array linking events to listeners functions in _listenersX (16 events, 16 listeners per event)
	uint32_t _eventListenerCount[events] = { 0 };
	uint32_t _eventListeners[events][listeners] = { { 0 } };

	// used for back-referencing object pointer to event
	std::unordered_map<void*, std::unordered_map<uint32_t, EventLink>> _eventLinks;

	// compile time recursive function for calling std::bind with generic arguments
	template <class ...Ts, typename T, int... indices>
	inline std::function<void(Ts...)> _bind(void (T::*func)(Ts...), T* obj, std::integer_sequence<int, indices...>);

	template <class ...Ts, typename T>
	inline std::function<void(Ts...)> _bind(void (T::*func)(Ts...), T* obj);

public:
	// clean-up dynamic memory in BaseFunctions
	~EventHandler();
	
	// subscribe member function to event number, the lower the order number the sooner it will be called
	template <typename T, typename ...Ts>
	inline void subscribe(T* obj, int event, void (T::*func)(Ts...), int32_t order = 0);

	// ubsubscribe specific member function from event
	template <typename T>
	inline void unsubscribe(T* obj, int event);

	// unsubscribe specific object from all events
	template <typename T>
	inline void unsubscribe(T* obj);

	// unsubscribe all object from specific event
	inline void unsubscribe(int event);

	// dispatch an event with arguments
	template <typename ...Ts>
	inline void dispatch(int event, Ts&&... args);
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

template<uint32_t events, uint32_t listeners>
inline EventHandler<events, listeners>::~EventHandler(){
	for (uint32_t i = 0; i < events * listeners; i++) {
		if (_listenerData[i].function == std::nullopt)
			continue;

		// clean up dynamic memory in BaseFunctions
		_listenerData[i].function->~BaseFunction();
	}
}

template <uint32_t events, uint32_t listeners>
template <typename T, typename ...Ts>
void EventHandler<events, listeners>::subscribe(T* obj, int event, void (T::*func)(Ts...), int32_t order) {
	// if object already subsribed to event, unsubscribe it
	if (_eventLinks[obj].find(event) != _eventLinks[obj].end())
		unsubscribe(obj, event);

	// find new index for access to a new Listener object in _listenerData
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
	assert(_eventListenerCount[event] < listeners && "no more listener slots");

	// store listener information for later re-organizing or unsubscribing
	new (&_listenerData[index].function) std::optional<Function<std::function<void(Ts...)>>>(_bind(func, obj));
	_listenerData[index].object = obj;
	_listenerData[index].order = order;

	// find appropriate slot in the _eventListeners array to place new listener index ordered by order, last slot by default
	uint32_t lastListener = _eventListenerCount[event] - 1;
	uint32_t listener = lastListener + 1;

	for (uint32_t i = 0; i <= lastListener; i++) {
		uint32_t index = _eventListeners[event][i];

		if (order < _listenerData[index].order)
			continue;

		listener = i;
		break;
	}

	// map the object pointer and event to the listener index for later re-organizing or unsubscribing
	_eventLinks[obj][event] = { index, listener };

	// if not the last listener, copy over other indexes by 1 to accomidate new index
	if (listener != lastListener + 1) {
		memcpy(&_eventListeners[event][listener + 1], &_eventListeners[event][listener], sizeof(uint32_t) * ((lastListener + 1) - listener));
	
		// update moved indexes in _eventLinks
		for (uint32_t i = listener + 1; i <= lastListener + 1; i++) {
			void* object = _listenerData[_eventListeners[event][i]].object;
			_eventLinks[object][event].listener++;
		}
	}

	// subscribe the listener to an event by putting its index in the _eventLinks map
	_eventListeners[event][listener] = index;
	_eventListenerCount[event]++;
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj, int event) {
	assert(event < events && "invalid event slot");

	// if object isn't subsribed to event, return
	if (_eventLinks[obj].find(event) == _eventLinks[obj].end())
		return;

	assert(_eventListenerCount[event]); // sanity
	assert(_listenerData[_eventLinks[obj][event].index].function.has_value()); // sanity

	// get listener index data from map
	uint32_t listener = _eventLinks[obj][event].listener;
	uint32_t index = _eventLinks[obj][event].index;

	// destroy optional BaseFunction object and free the index
	_listenerData[index].function->~BaseFunction();
	_listenerData[index].function = std::nullopt;

	_freeIndexes.push_back(index);

	// remove the event from the map
	_eventLinks[obj].erase(event);

	// remove the index from the _eventListeners
	uint32_t lastListener = _eventListenerCount[event] - 1;

	// if not the last listener, copy over following indexes by 1 to cover up empty slot
	if (lastListener != listener) {
		memcpy(&_eventListeners[event][listener], &_eventListeners[event][listener + 1], sizeof(uint32_t) * (lastListener - listener));
	
		// update moved indexes in _eventLinks
		for (uint32_t i = listener; i <= lastListener - 1; i++) {
			void* object = _listenerData[_eventListeners[event][i]].object;
			_eventLinks[object][event].listener--;
		}
	}

	// update listener count for event
	_eventListenerCount[event]--;
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj) {
	// if object has no events, return
	if (_eventLinks[obj].size() == 0)
		return;

	// copy indexes so they don't change during unsubsribe loop
	std::vector<uint32_t> events;
	events.reserve(_eventLinks[obj].size());

	for (const auto& i : _eventLinks[obj])
		events.push_back(i.first);

	// unsubscribe loop using copied indexes
	for (uint32_t i : events) 
		unsubscribe(obj, i);
}

template<uint32_t events, uint32_t listeners>
void EventHandler<events, listeners>::unsubscribe(int event){
	if (_eventListenerCount[event] == 0) // check
		return;

	// copy indexes so they don't change during unsubsribe loop
	std::vector<uint32_t> indexes;
	indexes.reserve(_eventListenerCount[event]);

	for (uint32_t i = 0; i < _eventListenerCount[event]; i++)
		indexes.push_back(_eventListeners[event][i]);

	// unsubscribe loop using copied indexes
	for (uint32_t i : indexes)
		unsubscribe(_listenerData[i].object, event);
}

template <uint32_t events, uint32_t listeners>
template <typename ...Ts>
void EventHandler<events, listeners>::dispatch(int event, Ts&&... args) {
	assert(event < events && "invalid event slot");

	// if no events, return
	if (_eventListenerCount[event] == 0)
		return;

	// copy indexes in case an event causes a subscribe or unsubsribe
	std::vector<uint32_t> indexes;
	indexes.reserve(_eventListenerCount[event]);

	for (int32_t i = _eventListenerCount[event] - 1; i >= 0; i--)
		indexes.push_back(_eventListeners[event][i]);

	// dispatch loop
	for (uint32_t i : indexes)
		_listenerData[i].function->call(std::forward<Ts>(args)...);
}