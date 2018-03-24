#include "Physics.hpp"

#include "Collider.hpp"
#include "Transform.hpp"

Physics::Physics(Engine & engine) : _engine(engine){
	_engine.events.subscribe(this, Events::Load, &Physics::load);
	_engine.events.subscribe(this, Events::Update, &Physics::update);
}

Physics::~Physics(){
	_engine.entities.iterate<Collider>([](uint64_t id, Collider& collider) {
		delete collider.collisionShape;
		delete collider.rigidBody;
	});

	delete _dynamicsWorld;
	delete _solver;
	delete _overlappingPairCache;
	delete _dispatcher;
	delete _collisionConfiguration;
}

void Physics::load(int argc, char ** argv){
	_collisionConfiguration = new btDefaultCollisionConfiguration();

	_dispatcher = new btCollisionDispatcher(_collisionConfiguration);

	_overlappingPairCache = new btDbvtBroadphase();

	_solver = new btSequentialImpulseConstraintSolver;

	_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher, _overlappingPairCache, _solver, _collisionConfiguration);

	setGravity({ 0.f, 0.f, 0.f });
}

void Physics::update(double dt){
	for (uint32_t i = 0; i < STEPS_PER_UPDATE; i++)
		_dynamicsWorld->stepSimulation(static_cast<float>(dt) / STEPS_PER_UPDATE, 0);
}

void Physics::createRigidBody(uint64_t id, glm::vec3 size, float mass){
	_engine.entities.add<Transform>(id);
	_engine.entities.add<Collider>(id);
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);
	
	collider->collisionShape = new btBoxShape(btVector3(btScalar(size.x), btScalar(size.y), btScalar(size.z)));

	btVector3 localInertia(0, 0, 0);

	if (mass != 0.f)
		collider->collisionShape->calculateLocalInertia(static_cast<btScalar>(mass), localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(static_cast<btScalar>(mass), transform, collider->collisionShape, localInertia);
	collider->rigidBody = new btRigidBody(rbInfo);

	_dynamicsWorld->addRigidBody(collider->rigidBody);
}

void Physics::setGravity(glm::vec3 direction){
	if (!_dynamicsWorld)
		return;

	_dynamicsWorld->setGravity(btVector3(
		static_cast<btScalar>(direction.x), 
		static_cast<btScalar>(direction.y), 
		static_cast<btScalar>(direction.z)
	));
}