#pragma once

#include "Config.hpp"

#include <btBulletDynamicsCommon.h>

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

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

	struct SweepHit {
		uint64_t id = 0;
		glm::dvec3 position;
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

	void sphereTest(float radius, const glm::dvec3& position, const glm::dquat& rotation, std::vector<SweepHit>& hits);
	//void boxTest(const glm::dvec3& dimensions, const glm::dvec3& position, const glm::dquat& rotation);
	//void cylinderTest(float radius,  float height, const glm::dvec3& position, const glm::dquat& rotation);
	//void capsuleTest(float radius, float height, const glm::dvec3& position, const glm::dquat& rotation);
	//void meshTest(const std::string& file, const glm::dvec3& position, const glm::dquat& rotation);
	//void planeTest(const glm::dvec3& position, const glm::dquat& rotation);

	void sphereSweep(uint64_t id, float radius, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot, std::vector<SweepHit>& hits);
	//void boxSweep(const glm::dvec3& dimensions, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRotn);
	//void cylinderSweep(float radius,  float height, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot);
	//void capsuleSweep(float radius, float height, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot);
	//void meshSweep(const std::string& file, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot);
	//void planeSweep(const glm::dvec3& position, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot);

	friend class Collider;
};