#include "Physics.hpp"

#include "Collider.hpp"
#include "Transform.hpp"

#include <BulletCollision\NarrowPhaseCollision\btRaycastCallback.h>

void Physics::_addCollider(uint64_t id, btScalar mass, btCollisionShape* shape) {
	shape->setUserPointer(_engine.entities.add<Transform>(id));
	_engine.entities.add<Collider>(id, shape, mass);
	updateCompoundShape(id);
}

void Physics::_createRigidBody(uint64_t id, btScalar mass, const btVector3& localInertia){
	Collider* collider = _engine.entities.get<Collider>(id);

	assert(collider && !collider->_rigidBody && collider->_compoundShape);

	collider->_rigidBody = new btRigidBody(
		mass,
		_engine.entities.get<Transform>(id),
		collider->_compoundShape,
		localInertia
	);

	collider->_rigidBody->setUserPointer(_engine.entities.get<Transform>(id));

	_dynamicsWorld->addRigidBody(collider->_rigidBody);
}

void Physics::_recursiveGetMasses(uint64_t id, std::vector<btScalar>* masses){
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (!collider)
		return;

	masses->push_back(collider->_mass);

	for (uint32_t i = 0; i < transform->children(); i++)
		_recursiveGetMasses(transform->child(i), masses);
}

void Physics::_recursiveUpdateCompoundShape(uint64_t id, std::vector<btScalar>* masses) {
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (!collider)
		return;

	if (collider->_rigidBody) {
		_dynamicsWorld->removeRigidBody(collider->_rigidBody);
		delete collider->_rigidBody;
		collider->_rigidBody = nullptr;
	}

	if (collider->_compoundShape)
		delete collider->_compoundShape;

	collider->_centerOfMass = glm::dvec3();
	collider->_compoundShape = new btCompoundShape();

	collider->_compoundShape->setUserPointer(transform);

	btTransform identityTransform;
	identityTransform.setIdentity();

	collider->_compoundShape->addChildShape(identityTransform, collider->_collisionShape);

	if (collider->_mass == 0.0)
		masses->push_back(1.0);
	else
		masses->push_back(collider->_mass);

	for (uint32_t i = 0; i < transform->children(); i++)
		_recursiveUpdateCompoundShape(transform->child(i), masses);

	Collider* parentCollider = _engine.entities.get<Collider>(transform->parent());

	if (parentCollider) {
		btTransform localTransform;
		localTransform.setOrigin(toBt(transform->position()));
		localTransform.setRotation(toBt(transform->rotation()));

		collider->_compoundIndex = parentCollider->_compoundShape->getNumChildShapes();
		parentCollider->_compoundShape->addChildShape(localTransform, collider->_compoundShape);
	}
}

void Physics::_removeFromWorld(uint64_t id){
	Collider* collider = _engine.entities.get<Collider>(id);

	assert(collider && _dynamicsWorld && collider->_rigidBody);

	_dynamicsWorld->removeRigidBody(collider->_rigidBody);
}

void Physics::_setCenterOfMass(uint64_t id, const btVector3& position) {
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (!collider || transform->parent())
		return;

	std::cout << position.x() << ' ' << position.y() << ' ' << position.z() << '\n';

	glm::dvec3 difference = transform->rotation() * toGlm<double>(position) - transform->rotation() * collider->_centerOfMass;

	for (uint32_t i = 0; i < collider->_compoundShape->getNumChildShapes(); i++) {
		Transform* child = static_cast<Transform*>(collider->_compoundShape->getChildShape(i)->getUserPointer());

		btTransform offset;

		if (i) {
			offset.setOrigin(toBt(child->position()) - position);
			offset.setRotation(toBt(child->rotation()));
		}
		else {
			offset.setIdentity();
			offset.setOrigin(-position);
		}

		collider->_compoundShape->updateChildTransform(i, offset);
	}

	collider->_centerOfMass = toGlm<double>(position);

	btTransform rootOffset = collider->_rigidBody->getWorldTransform();
	rootOffset.setOrigin(rootOffset.getOrigin() + toBt(difference));

	collider->_rigidBody->setWorldTransform(rootOffset);
	collider->_rigidBody->setInterpolationWorldTransform(rootOffset);



	
	//btTransform centerOfMass;
	//centerOfMass.setOrigin(toBt(transform->worldPosition()) + position);
	//centerOfMass.setRotation(toBt(transform->worldRotation()));
	//
	//collider->_rigidBody->setCenterOfMassTransform(centerOfMass);

	//transform->setPosition(transform->worldPosition() + collider->_centerOfMass);

	//updateWorldTransform(id);

	// collider should apply centerofmass during updateWorldTransform
	// call it here instead of applying offset manually
}

uint64_t Physics::_rootCollider(uint64_t id) const {
	if (!_engine.entities.has<Collider>(id))
		return 0;

	uint64_t root = id;
	Transform* i = _engine.entities.get<Transform>(id);

	while (i && i->parent()) {
		if (!_engine.entities.has<Collider>(i->parent()))
			break;

		root = i->parent();
		i = _engine.entities.get<Transform>(i->parent());
	}

	return root;
}

Physics::Physics(Engine & engine) : _engine(engine){
	_engine.events.subscribe(this, Events::Load, &Physics::load);
	_engine.events.subscribe(this, Events::Update, &Physics::update);
}

Physics::~Physics(){
	if (!_dynamicsWorld)
		return;

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

	setGravity(glm::dvec3());
}

void Physics::update(double dt){
	for (uint32_t i = 0; i < DEFUALT_PHYSICS_STEPS; i++)
		_dynamicsWorld->stepSimulation(static_cast<btScalar>(dt) / DEFUALT_PHYSICS_STEPS, 0);
}

void Physics::updateWorldTransform(uint64_t id) {
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (!collider)
		return;

	//std::cout << transform->position().x << ' ' << transform->position().y << ' ' << transform->position().z << '\n';

	Collider* parentCollider = _engine.entities.get<Collider>(transform->parent());

	if (parentCollider) {
		uint64_t root = _rootCollider(id);
		Collider* rootCollider = _engine.entities.get<Collider>(root);

		btTransform localTransform;
		localTransform.setOrigin(toBt(transform->position() - rootCollider->_centerOfMass));
		localTransform.setRotation(toBt(transform->rotation()));
		
		parentCollider->_compoundShape->updateChildTransform(collider->_compoundIndex, localTransform);
		

		for (uint32_t i = 0; i < parentCollider->_compoundShape->getNumChildShapes(); i++) {
			btTransform childTransform = parentCollider->_compoundShape->getChildTransform(i);
		
			childTransform.setOrigin(childTransform.getOrigin() + toBt(rootCollider->centerOfMass()));

			parentCollider->_compoundShape->updateChildTransform(i, childTransform);
		}

		
		std::vector<btScalar> masses;
		_recursiveGetMasses(root, &masses);
		
		btTransform principal;
		principal.setIdentity();
		
		btVector3 localInertia = rootCollider->_rigidBody->getLocalInertia();
		
		rootCollider->_compoundShape->calculatePrincipalAxisTransform(&*masses.begin(), principal, localInertia);
		
		_setCenterOfMass(root, principal.getOrigin());



		//Transform* rootTransform = _engine.entities.get<Transform>(root);

		//glm::dvec3 position = rootTransform->position();
		//std::cout << position.x << ' ' << position.y << ' ' << position.z << '\n';
		

		//btTransform rootOffset = rootCollider->_rigidBody->getWorldTransform();
		//
		//rootOffset.setOrigin(rootOffset.getOrigin() + btVector3(0, 0, 100));
		//
		//rootCollider->_rigidBody->setInterpolationWorldTransform(rootOffset);
	}
	else {
		btTransform worldTransform;
		worldTransform.setOrigin(toBt(transform->worldPosition() + transform->worldRotation() * collider->_centerOfMass));
		worldTransform.setRotation(toBt(transform->worldRotation()));

		collider->_rigidBody->setWorldTransform(worldTransform);
	}
}

void Physics::updateCompoundShape(uint64_t id) {
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (!collider)
		return;

	// find root collider
	uint64_t root = _rootCollider(id);

	// from root, update all children's compound and shape and get rid of their rigid bodies
	std::vector<btScalar> masses;
	_recursiveUpdateCompoundShape(root, &masses);

	// create root collider's rigidbody with offset for center of mass
	Transform* rootTransform = _engine.entities.get<Transform>(root);
	Collider* rootCollider = _engine.entities.get<Collider>(root);

	btTransform principal;
	principal.setIdentity();

	btVector3 localInertia;

	btScalar mass = 0;

	if (rootCollider->_mass != 0.0) {
		for (btScalar i : masses)
			mass += i;
	}

	if (rootTransform->children() && mass != 0)
		rootCollider->_compoundShape->calculatePrincipalAxisTransform(&*masses.begin(), principal, localInertia);
	else if (mass != 0)
		rootCollider->_collisionShape->calculateLocalInertia(rootCollider->_mass, localInertia);

	_createRigidBody(root, mass, localInertia);

	if (rootTransform->children() && mass != 0) {
		_setCenterOfMass(root, principal.getOrigin());

		//rootCollider->_compoundShape->calculatePrincipalAxisTransform(&*masses.begin(), principal, localInertia);
		//_setCenterOfMass(root, principal.getOrigin());



	}

	//updateWorldTransform(root);

	//if (rootTransform->children() && mass != 0) {
	//	glm::dvec3 position = rootTransform->position();
	//	std::cout << position.x << ' ' << position.y << ' ' << position.z << '\n';
	//}
}

void Physics::setGravity(const glm::dvec3& direction){
	if (!_dynamicsWorld)
		return;

	_dynamicsWorld->setGravity(toBt(direction));
}

void Physics::addSphere(uint64_t id, float radius, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addCollider(id, mass, new btSphereShape(static_cast<btScalar>(radius)));
}

void Physics::addBox(uint64_t id, const glm::dvec3& dimensions, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addCollider(id, mass, new btBoxShape(btVector3(dimensions.x, dimensions.y, dimensions.z) * 2));
}

void Physics::addCylinder(uint64_t id, float radius, float height, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addCollider(id, mass, new btCylinderShape(btVector3(radius * 2, radius * 2, height)));
}

void Physics::addCapsule(uint64_t id, float radius, float height, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addCollider(id, mass, new btCapsuleShape(radius, height));
}

void Physics::addStaticPlane(uint64_t id){
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addCollider(id, 0.f, new btStaticPlaneShape(btVector3(0, 0, 1), 0));
}

void Physics::rayTest(const glm::dvec3 & from, const glm::dvec3 & to, std::vector<RayHit>& hits){
	btVector3 bFrom(toBt(from));
	btVector3 bTo(toBt(to));

	btCollisionWorld::AllHitsRayResultCallback results(bFrom, bTo);

	results.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
	results.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

	_dynamicsWorld->rayTest(bFrom, bTo, results);

	hits.reserve(results.m_collisionObjects.size());

	for (uint32_t i = 0; i < results.m_collisionObjects.size(); i++) {
		uint64_t id = static_cast<Transform*>(results.m_collisionObjects[i]->getUserPointer())->id();

		hits.push_back({ id, toGlm<double>(results.m_hitPointWorld[i]), toGlm<double>(results.m_hitNormalWorld[i]) });
	}
}

Physics::RayHit Physics::rayTest(const glm::dvec3 & from, const glm::dvec3 & to){
	btVector3 bFrom(toBt(from));
	btVector3 bTo(toBt(to));

	btCollisionWorld::ClosestRayResultCallback results(bFrom, bTo);

	results.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
	results.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

	_dynamicsWorld->rayTest(bFrom, bTo, results);

	if (!results.hasHit())
		return RayHit();

	uint64_t id = static_cast<Transform*>(results.m_collisionObject->getUserPointer())->id();

	return { id, toGlm<double>(results.m_hitPointWorld), toGlm<double>(results.m_hitNormalWorld) };
}

void Physics::sphereTest(float radius, const glm::dvec3 & position, const glm::dquat & rotation, std::vector<SweepHit>& hits){

}

void Physics::sphereSweep(uint64_t id, float radius, const glm::dvec3 & fromPos, const glm::dquat & fromRot, const glm::dvec3 & toPos, const glm::dvec3 & toRot, std::vector<SweepHit>& hits){

}
