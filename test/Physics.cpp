#include "Physics.hpp"

#include "Collider.hpp"
#include "Transform.hpp"

Physics::Physics(Engine & engine) : _engine(engine){
	_engine.events.subscribe(this, Events::Load, &Physics::load);
	_engine.events.subscribe(this, Events::Update, &Physics::update);
}

Physics::~Physics(){
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}

void Physics::load(int argc, char ** argv){
	collisionConfiguration = new btDefaultCollisionConfiguration();

	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	overlappingPairCache = new btDbvtBroadphase();

	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, 0, -100));
}

void Physics::update(double dt){
	dynamicsWorld->stepSimulation(static_cast<float>(dt), 10);

	for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;

		if (body && body->getMotionState())
			body->getMotionState()->getWorldTransform(trans);
		else
			trans = obj->getWorldTransform();

		Transform* transform = (Transform*)body->getUserPointer();

		transform->position.x = static_cast<double>(trans.getOrigin().getX());
		transform->position.y = static_cast<double>(trans.getOrigin().getY());
		transform->position.z = static_cast<double>(trans.getOrigin().getZ());

		transform->rotation.w = static_cast<float>(trans.getRotation().getW());
		transform->rotation.x = static_cast<float>(trans.getRotation().getX());
		transform->rotation.y = static_cast<float>(trans.getRotation().getY());
		transform->rotation.z = static_cast<float>(trans.getRotation().getZ());
	}
}

void Physics::createRigidBody(uint64_t id, glm::vec3 size, float mass){
	_engine.entities.add<Transform>(id);
	Transform* transform = _engine.entities.get<Transform>(id);
	
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(size.x), btScalar(size.y), btScalar(size.z)));

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(transform->position.x, transform->position.y, transform->position.z));
	groundTransform.setRotation(btQuaternion(transform->rotation.x, transform->rotation.y, transform->rotation.z, transform->rotation.w));

	btScalar bMass(mass);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (bMass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		groundShape->calculateLocalInertia(bMass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(bMass, myMotionState, groundShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setUserPointer(transform);

	//add the body to the dynamics world
	dynamicsWorld->addRigidBody(body);
}
