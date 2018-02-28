#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <set>
#include <optional>

//const uint32_t events = 16;
//const uint32_t listeners = 16;

namespace SimpleEngine{

template <uint32_t events, uint32_t listeners>
class EventHandler {
	/*
	struct BaseFunction {
		void* const ptr;

		inline BaseFunction(void* func) : ptr(func) {}

		inline virtual ~BaseFunction() {}
	};

	template <typename ...Ts>
	struct Function : public BaseFunction {
		static std::function<void(Ts...)> functions[16 * 16]; // dynamic memory maybe?
		// free index maybe?
		static uint32_t counter;

		const uint32_t count;

		inline Function(const std::function<void(Ts...)>& func);

		inline ~Function();
	};
	*/

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

	std::optional<BaseFunction> _functions[events * listeners] = { }; // Array of BaseFunctions (16 events * 16 listeners per event)
	void* _objects[events * listeners] = { nullptr };

	uint32_t _functionIndexes = 0;
	std::vector<uint32_t> _freeIndexes;

	uint32_t _listenerCount[events] = { 0 };
	uint32_t _eventListeners[events][listeners] = { { 0 } }; // 2D array linking events to listeners functions in _listenersX (16 events, 16 listeners per event)

	std::unordered_map<void*, std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>>> _eventLinks;

	//std::unordered_map<void*, std::tuple<uint32_t, uint32_t, uint32_t>> _ptrMap; // object pointer map to Function, Event, and Listener numbers

	//std::unordered_map<int, std::set<BaseFunction*>> _listeners;
	//std::unordered_map<void*, std::unordered_map<int, BaseFunction*>> _eventLinks;

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

	//inline void unsubscribe(int event);

	template <typename ...Ts>
	inline void dispatch(int event, Ts... args);
};

/*
template <typename ...Ts>
std::function<void(Ts...)> EventHandler::Function<Ts...>::functions[16 * 16] = { };

template <typename ...Ts>
uint32_t EventHandler::Function<Ts...>::counter = 0;

template <typename ...Ts>
EventHandler::Function<Ts...>::Function(const std::function<void(Ts...)>& func) : BaseFunction((void*)&functions[counter]), count(counter) {
	functions[count] = func;
	counter++;
}

template<typename ...Ts>
inline EventHandler::Function<Ts...>::~Function(){ }
*/

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
	/*
	//BaseFunction* instance = new Function<Ts...>(_bind(func, obj));
	BaseFunction* instance = new Function<std::function<void(Ts...)>>(_bind(func, obj));

	_listeners[event].insert(instance);
	//_eventLinks[obj][event] = instance;
	*/

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

	new (&_functions[index]) Function<std::function<void(Ts...)>>(_bind(func, obj));
	_objects[index] = (void*)obj;

	_eventListeners[event][_listenerCount[event]] = index;
	_listenerCount[event]++;
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj, int event) {
	assert(event < events && "invalid event slot");

	uint32_t index = _eventLinks[obj][event].first;
	uint32_t listener = _eventLinks[obj][event].second;

	/*BaseFunction* instance = _eventLinks[obj][event];

	_listeners[event].erase(instance);

	if (!_listeners[event].size())
		_listeners.erase(event);

	_eventLinks[obj].erase(event);

	delete instance;*/
}

template <uint32_t events, uint32_t listeners>
template <typename T>
void EventHandler<events, listeners>::unsubscribe(T* obj) {
	/*for (auto i : _eventLinks[obj]) {
		_listeners[i.first].erase(i.second);

		if (!_listeners[i.first].size())
			_listeners.erase(i.first);

		delete i.second;
	}

	_eventLinks.erase(obj);*/
}

template <uint32_t events, uint32_t listeners>
template <typename ...Ts>
void EventHandler<events, listeners>::dispatch(int event, Ts... args) {
	/*
	for (BaseFunction* instance : _listeners[event]) {
		(*static_cast<std::function<void(Ts...)>*>(instance->ptr))(args...);
	}
	*/

	assert(event < events && "invalid event slot");

	for (uint32_t i = 0; i < _listenerCount[event]; i++) {
		_functions[_eventListeners[event][i]]->call(args...);
	}
}

}