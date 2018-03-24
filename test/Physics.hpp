#pragma once

#include "Config.hpp"

#include <btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>

#include <string>
#include <unordered_map>

#define STEPS_PER_UPDATE 10

class Physics {
	Engine& _engine;

	btDefaultCollisionConfiguration* _collisionConfiguration;

	btCollisionDispatcher* _dispatcher;

	btBroadphaseInterface* _overlappingPairCache;

	btSequentialImpulseConstraintSolver* _solver;

	btDiscreteDynamicsWorld* _dynamicsWorld;

public:
	Physics(Engine& engine);
	~Physics();

	void load(int argc, char** argv);
	void update(double dt);

	void createRigidBody(uint64_t id, glm::vec3 size, float mass);
	
	void setGravity(glm::vec3 direction);
	
	void addSphere(float radius, float mass = 0.f, glm::vec3 origin = glm::vec3()); // btSphereShape
	void addBox(glm::vec3 dimensions, float mass = 0.f, glm::vec3 origin = glm::vec3()); // btBoxShape
	void addCylinder(float radius,  float height, float mass = 0.f, glm::vec3 origin = glm::vec3()); // btCylinderShape
	void addCapsule(float radius, float height, float mass = 0.f, glm::vec3 origin = glm::vec3()); // btCapsuleShape
	void addEllipsoid(glm::vec3 dimensions, float mass = 0.f, glm::vec3 origin = glm::vec3()); // btMultiSphereShape
	
	void addDynamicMesh(const std::string& file, float mass, glm::vec3 origin = glm::vec3()); // btConvexPointCloudShape
	
	void addStaticMesh(const std::string& file, glm::vec3 origin = glm::vec3()); // btBvhTriangleMeshShape
	void addStaticPlane(glm::vec3 origin = glm::vec3()); // btStaticPlaneShape
	void addStaticHeightmap(const std::string& file, glm::vec3 origin = glm::vec3()); // btHeightfieldTerrainShape
};