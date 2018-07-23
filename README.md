# Simple Engine

```cpp
#include <SimpleEngine.hpp>

/*
	User must define interface class derived from 'SimpleEngine<YourInterfaceClass, 8>::BaseSystem', 8 being the maximum amount of components an Entity can have.
	Shared functionality should be defined here, and overridden in systems.
*/
class SystemInterface : public SimpleEngine<SystemInterface, 8>::BaseSystem {
public:
	virtual void initiate(int argc, char** argv) {}
	virtual void update(float dt) {}
};

/*
	Components do not have to be derived from anything and can use constructors and destructors.
*/
struct Transform {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	Transform(float x, float y, float z) : x(x), y(y), z(z) {	}
};

/*
	User defined systems must be derived from the user defined interface class.
	Passing 'Engine& engine' in the constructor isn't required, although necassary if you want to manipulate entities.
*/
class MySystem : public SystemInterface{
	Engine& _engine;

	Engine::Entity _myEntity;
	std::vector<Engine::Entity> _myEntities;

public:
	/*
		Overridden interface functions aren't called by default, using SYSFUNC_ENABLE enables them and sets their call priority relative to other systems.
		A system with a function priority set at -1 will be called before another system's function priority set at 0.
	*/
	MySystem(Engine& engine) : _engine(engine), _myEntity(engine) {
		SYSFUNC_ENABLE(SystemInterface, initiate, 0);
		SYSFUNC_ENABLE(SystemInterface, update, 0);
	}

	void initiate(int argc, char** argv) override {
		/*
			Creating an entity using ID only. Doesn't create a reference and can be invalidated from elsewhere.
		*/
		uint64_t id = _engine.createEntity();
		_engine.addComponent<Transform>(id, 1, 2, 3);

		_engine.destroyEntity(id);

		/*
			Storing an entity id in an Entity container. Creates a reference and can't be invalidated until object is relinquished.
		*/
		_myEntity.set(_engine.createEntity());
		_myEntity.add<Transform>(1, 2, 3);

		_myEntity.destroy();

		/*
			Storing entities in vector.
		*/
		_myEntities.resize(10, _engine);

		for (Engine::Entity& entity : _myEntities) {
			entity.set(_engine.createEntity());
			entity.add<Transform>(1, 2, 3);
		}
	}

	/*
		Iterating over entities is done using a lambda taking 'Engine::Entity& entity'.
	*/
	void update(float dt) override {
		_engine.iterateEntities([&](Engine::Entity& entity) {
			if (!entity.has<Transform>())
				return;

			Transform& transform = *entity.get<Transform>();

			// etc...
		});
	}
};

int main(int argc, char** argv) {
	/*
		Engine is created with desired chunk size, which is the amount of memory that will be allocated when making memory for new entities.
		Dependent on available memory and game type. Higher means better performance creating bulk entities, at the expense of more memory usage.
	*/
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	/*
		Systems should be registered and passed their desired construction args.
	*/
	engine.registerSystem<MySystem>(engine);

	/*
		SYSFUNC_CALL calls a given system interface function on all registered systems.
	*/
	SYSFUNC_CALL(SystemInterface, initiate, engine)(argc, argv);

	TimePoint timer;
	float dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, update, engine)(dt);

		dt = deltaTime<float>(timer);
	}

	return 0;
}
```