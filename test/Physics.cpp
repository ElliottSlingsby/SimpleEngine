#include "Physics.hpp"

#include "Collider.hpp"
#include "Transform.hpp"

void Physics::addRigidBody(uint64_t id, float mass, btCollisionShape* shape){
	Transform& transform = *_engine.entities.add<Transform>(id);
	Collider& collider = *_engine.entities.add<Collider>(id);

	if (collider.collisionShape)
		delete collider.collisionShape;

	if (collider.rigidBody) {
		_dynamicsWorld->removeRigidBody(collider.rigidBody);
		delete collider.rigidBody;
	}
	
	collider.collisionShape = shape;

	btVector3 localInertia(0, 0, 0);

	if (mass != 0.f)
		collider.collisionShape->calculateLocalInertia(static_cast<btScalar>(mass), localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(static_cast<btScalar>(mass), &transform, collider.collisionShape, localInertia);
	collider.rigidBody = new btRigidBody(rbInfo);

	_dynamicsWorld->addRigidBody(collider.rigidBody);
}

Physics::Physics(Engine & engine) : _engine(engine){
	_engine.events.subscribe(this, Events::Load, &Physics::load);
	_engine.events.subscribe(this, Events::Update, &Physics::update);
}

Physics::~Physics(){
	_engine.entities.iterate<Collider>([&](uint64_t id, Collider& collider) {
		delete collider.collisionShape;

		_dynamicsWorld->removeRigidBody(collider.rigidBody);
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
	for (uint32_t i = 0; i < DEFUALT_PHYSICS_STEPS; i++)
		_dynamicsWorld->stepSimulation(static_cast<float>(dt) / DEFUALT_PHYSICS_STEPS, 0);
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

void Physics::addSphere(uint64_t id, float radius, float mass) {
	addRigidBody(id, mass, new btSphereShape(static_cast<btScalar>(radius)));
}

void Physics::addBox(uint64_t id, glm::vec3 dimensions, float mass) {
	addRigidBody(id, mass, new btBoxShape(btVector3(dimensions.x, dimensions.y, dimensions.z) * 2));
}

void Physics::addCylinder(uint64_t id, float radius, float height, float mass) {
	addRigidBody(id, mass, new btCylinderShape(btVector3(radius * 2, radius * 2, height)));
}

void Physics::addCapsule(uint64_t id, float radius, float height, float mass) {
	addRigidBody(id, mass, new btCapsuleShape(radius, height));
}

void Physics::addStaticPlane(uint64_t id){
	addRigidBody(id, 0.f, new btStaticPlaneShape(btVector3(0, 0, 1), 0));
}
