#include "Physics.hpp"

#include "Collider.hpp"
#include "Transform.hpp"

#include <BulletCollision\NarrowPhaseCollision\btRaycastCallback.h>

void Physics::_addRigidBody(uint64_t id, float mass, btCollisionShape* shape){
	if (!_dynamicsWorld)
		return;

	Transform& transform = *_engine.entities.add<Transform>(id);

	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	Collider& collider = *_engine.entities.add<Collider>(id, shape, mass);
}

void Physics::_register(btRigidBody * rigidBody){
	if (_dynamicsWorld)
		_dynamicsWorld->addRigidBody(rigidBody);
}

void Physics::_unregister(btRigidBody * rigidBody) {
	if (_dynamicsWorld)
		_dynamicsWorld->removeRigidBody(rigidBody);
}

void Physics::_recursiveUpdateWorldTransform(uint64_t id, uint64_t rootCollider){
	Transform* transform = _engine.entities.get<Transform>(id);
	Collider* collider = _engine.entities.get<Collider>(id);

	if (collider && (!collider->_compoundShape || collider->_rootCompound)) {
		// if the root of compoundshape or just a single rigidbody, then update position relative to world
		btTransform worldTransform = collider->_rigidBody->getWorldTransform();
		worldTransform.setOrigin(toBt(transform->worldPosition()));
		worldTransform.setRotation(toBt(transform->worldRotation()));

		collider->_rigidBody->setWorldTransform(worldTransform);
	}
	else if (collider){
		// otherwise must be a child of a compoundshape, then update position relative to root of compound shape
		btTransform relativeTransform;
		relativeTransform.setOrigin(toBt(transform->relativePosition(rootCollider)));
		relativeTransform.setRotation(toBt(transform->relativeRotation(rootCollider)));
		
		collider->_compoundShape->updateChildTransform(collider->_compoundIndex, relativeTransform, false);
	}

	// recurse
	for (uint32_t i = 0; i < transform->children(); i++) 
		_recursiveUpdateWorldTransform(transform->child(i), rootCollider);
}

void Physics::_recursiveUpdateCompoundShape(btCompoundShape* compoundShape, uint64_t id, uint64_t rootCollider, std::vector<float>* masses){
	// starting from root collider, and then recursively through children:
	//	do nothing and go deeper if no collider
	//	else
	//  add shape to collider and update compoundindex using relative transform to root collider
	//  unregister rigidbody if !compoundshape or compoundshape and rootcompound
	//  delete compoundshape if compoundshape and rootcompound and change rootcompound
	//  set compoundshape pointer to new compoundshape

	Transform* transform = _engine.entities.get<Transform>(id);

	// if no collider, then do nothing
	if (_engine.entities.has<Collider>(id)) {
		Collider* collider = _engine.entities.get<Collider>(id);

		// otherwise get rid of its rigidbody (no longer needed)
		if (collider->_rigidBody) {
			_dynamicsWorld->removeRigidBody(collider->_rigidBody);
			delete collider->_rigidBody;
			collider->_rigidBody = nullptr;
		}

		// delete its compoundshape if it was the root
		if (collider->_rootCompound) {
			collider->_rootCompound = false;
			delete collider->_compoundShape;
		}

		// get relative position to top most parent with collider
		btTransform relativeTransform;
		relativeTransform.setOrigin(toBt(transform->relativePosition(rootCollider)));
		relativeTransform.setRotation(toBt(transform->relativeRotation(rootCollider)));

		// add the shape to the compoundshape with local offset
		compoundShape->addChildShape(relativeTransform, collider->_collisionShape);

		collider->_compoundIndex = compoundShape->getNumChildShapes() - 1;
		collider->_compoundShape = compoundShape;

		masses->push_back(collider->_mass);
	}

	// recurse
	for (uint32_t i = 0; i < transform->children(); i++) 
		_recursiveUpdateCompoundShape(compoundShape, transform->child(i), rootCollider, masses);
}

bool Physics::_recursiveHasCollider(uint64_t id){
	Transform* transform = _engine.entities.get<Transform>(id);

	if (!transform)
		return false;

	if (_engine.entities.has<Collider>(id))
		return true;

	for (uint32_t i = 0; i < transform->children(); i++)
		if (_recursiveHasCollider(transform->child(i)))
			return true;

	return false;
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

void Physics::updateWorldTransform(uint64_t id){
	if (!_dynamicsWorld)
		return;

	Transform* transform = _engine.entities.get<Transform>(id);

	uint64_t rootCollider = 0;
	uint64_t i = id;

	while (i) {
		Collider* collider = _engine.entities.get<Collider>(i);

		if (collider && (collider->_rootCompound || !collider->_compoundShape)) {
			rootCollider = i;
			break;
		}

		i = transform->parent();
	}

	if (!rootCollider)
		return;

	_recursiveUpdateWorldTransform(id, rootCollider);

	Collider* collider = _engine.entities.get<Collider>(rootCollider);

	if (collider->_compoundShape)
		collider->_compoundShape->recalculateLocalAabb();
}

void Physics::updateCompoundShape(uint64_t id) {
	if (!_engine.entities.has<Transform, Collider>(id))
		return;

	Transform* transform = _engine.entities.get<Transform>(id);	
	Collider* collider = _engine.entities.get<Collider>(id);
	
	// if not in hierarchy, and hasn't been a compound shape, then do nothing
	if (!transform->parent() && !transform->children() && !collider->_compoundShape)
		return;

	// if not in hierarchy, but is a compound shape, revert back to normal shape
	if (!transform->parent() && !transform->children() && collider->_compoundShape) {
		// delete compound shape if was root
		if (collider->_rootCompound)
			delete collider->_compoundShape;

		// null compound shape so it doesn't get used
		collider->_compoundShape = nullptr;

		// delete rigidbody if exists
		if (collider->_rigidBody) {
			_dynamicsWorld->removeRigidBody(collider->_rigidBody);
			delete collider->_rigidBody;
		}

		// create new rigidbody
		btVector3 localInertia;

		if (collider->_mass != 0.f)
			collider->_collisionShape->calculateLocalInertia(collider->_mass, localInertia);

		collider->_rigidBody = new btRigidBody(collider->_mass, transform, collider->_collisionShape, localInertia);
		collider->_rigidBody->setUserPointer(transform);
		_dynamicsWorld->addRigidBody(collider->_rigidBody);

		return;
	}

	// if has collider in children 
	if (!_recursiveHasCollider(id))
		return;

	// find the top most entity parent with a collider
	std::vector<uint64_t> stack = { id };

	while (transform->parent()) {
		stack.push_back(transform->parent());
		transform = _engine.entities.get<Transform>(transform->parent());
	}

	while (stack.size() && !_engine.entities.has<Collider>(*stack.rbegin()))
		stack.pop_back();

	uint64_t root = *stack.rbegin();
	Transform* rootTransform = _engine.entities.get<Transform>(root);
	Collider* rootCollider = _engine.entities.get<Collider>(root);

	// build compound shape from children	
	btCompoundShape* compoundShape = new btCompoundShape();

	std::vector<float> masses;
	_recursiveUpdateCompoundShape(compoundShape, root, root, &masses);

	btVector3 localInertia;
	btTransform principal;

	if (rootCollider->_mass != 0.f)
		compoundShape->calculatePrincipalAxisTransform(&*masses.begin(), principal, localInertia);

	// create new rigidbody using new compound shape
	rootCollider->_rigidBody = new btRigidBody(rootCollider->_mass, rootTransform, compoundShape, localInertia);
	rootCollider->_rigidBody->setUserPointer(_engine.entities.get<Transform>(root));
	_dynamicsWorld->addRigidBody(rootCollider->_rigidBody);
	rootCollider->_rigidBody->setCenterOfMassTransform(principal);

	rootCollider->_rootCompound = true;

	// set position to be safe
	btTransform worldTransform;
	worldTransform.setOrigin(toBt(rootTransform->worldPosition()));
	worldTransform.setRotation(toBt(rootTransform->worldRotation()));

	rootCollider->_rigidBody->setWorldTransform(worldTransform);
	
	// get root collider

	// create new compoundshape

	// starting from root collider, and then recursively through children:
	//	do nothing and go deeper if no collider
	//	else
	//  add shape to collider and update compoundindex using relative transform to root collider
	//  unregister rigidbody if !compoundshape or compoundshape and rootcompound
	//  delete compoundshape if compoundshape and rootcompound and change rootcompound
	//  set compoundshape pointer to new compoundshape

	// set root collider's rigidobyd collision shape
	// set root collider as rootcompound
}

void Physics::setGravity(const glm::dvec3& direction){
	if (!_dynamicsWorld)
		return;

	_dynamicsWorld->setGravity(toBt(direction));
}

void Physics::addSphere(uint64_t id, float radius, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addRigidBody(id, mass, new btSphereShape(static_cast<btScalar>(radius)));
}

void Physics::addBox(uint64_t id, const glm::dvec3& dimensions, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addRigidBody(id, mass, new btBoxShape(btVector3(dimensions.x, dimensions.y, dimensions.z) * 2));
}

void Physics::addCylinder(uint64_t id, float radius, float height, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addRigidBody(id, mass, new btCylinderShape(btVector3(radius * 2, radius * 2, height)));
}

void Physics::addCapsule(uint64_t id, float radius, float height, float mass) {
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addRigidBody(id, mass, new btCapsuleShape(radius, height));
}

void Physics::addStaticPlane(uint64_t id){
	if (_engine.entities.has<Collider>(id))
		_engine.entities.remove<Collider>(id);

	_addRigidBody(id, 0.f, new btStaticPlaneShape(btVector3(0, 0, 1), 0));
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
