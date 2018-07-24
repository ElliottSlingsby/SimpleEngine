#pragma once

#include "SystemInterface.hpp"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class Collider : public btMotionState {
	//btCollisionObject _collisionObject;
	//btRigidBody _rigidBody;

public:
	void getWorldTransform(btTransform &worldTrans) const override;
	void setWorldTransform(const btTransform &worldTrans) override;
};

class Physics : public SystemInterface {
	Engine& _engine;

	btDefaultCollisionConfiguration* _collisionConfiguration = nullptr;
	btCollisionDispatcher* _dispatcher = nullptr;
	btBroadphaseInterface* _overlappingPairCache = nullptr;
	btSequentialImpulseConstraintSolver* _solver = nullptr;
	btDiscreteDynamicsWorld* _dynamicsWorld = nullptr;

public:
	Physics(Engine& engine);
	~Physics();

	void initiate(const std::vector<std::string>& args) override;
	void update(double dt) override;
};