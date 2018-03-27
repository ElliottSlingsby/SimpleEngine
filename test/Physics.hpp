#pragma once

#include "Config.hpp"

#include <btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>

#include <string>
#include <unordered_map>

#define DEFUALT_PHYSICS_STEPS 10

class Physics {
	Engine& _engine;

	btDefaultCollisionConfiguration* _collisionConfiguration;

	btCollisionDispatcher* _dispatcher;

	btBroadphaseInterface* _overlappingPairCache;

	btSequentialImpulseConstraintSolver* _solver;

	btDiscreteDynamicsWorld* _dynamicsWorld;

	void addRigidBody(uint64_t id, float mass, btCollisionShape* shape);

public:
	struct RayHit {
		uint64_t id = 0;
		glm::dvec3 position;
		glm::dvec3 normal;
	};

	Physics(Engine& engine);
	~Physics();

	void load(int argc, char** argv);
	void update(double dt);
	
	void setGravity(const glm::dvec3& direction);
	
	void addSphere(uint64_t id, float radius, float mass = 0.f); // btSphereShape
	void addBox(uint64_t id, const glm::dvec3& dimensions, float mass = 0.f); // btBoxShape
	void addCylinder(uint64_t id, float radius,  float height, float mass = 0.f); // btCylinderShape
	void addCapsule(uint64_t id, float radius, float height, float mass = 0.f); // btCapsuleShape
	
	//void addDynamicMesh(uint64_t id, const std::string& file, float mass); // btConvexPointCloudShape

	//void addStaticMesh(uint64_t id, const std::string& file); // btBvhTriangleMeshShape
	void addStaticPlane(uint64_t id); // btStaticPlaneShape
	//void addStaticHeightmap(uint64_t id, const std::string& file); // btHeightfieldTerrainShape

	void rayTest(const glm::dvec3& from, const glm::dvec3& to, std::vector<RayHit>& hits);
	RayHit rayTest(const glm::dvec3& from, const glm::dvec3& to);

	friend class Collider;
};