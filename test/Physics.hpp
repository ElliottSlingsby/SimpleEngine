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

	void _addCollider(uint64_t id, btScalar mass, btCollisionShape* shape);

	void _createRigidBody(uint64_t id, btScalar mass, const btVector3& localInertia);

	void _recursiveGetMasses(uint64_t id, std::vector<btScalar>* masses);

	void _recursiveUpdateCompoundShape(uint64_t id, std::vector<btScalar>* masses);

	void _removeFromWorld(uint64_t id);

	void _setCenterOfMass(uint64_t id, const btVector3& position);

	uint64_t _rootCollider(uint64_t id) const;

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

	void updateWorldTransform(uint64_t id);
	void updateCompoundShape(uint64_t id);
	
	void setGravity(const glm::dvec3& direction);
	
	void addSphere(uint64_t id, float radius, float mass = 0.f); // btSphereShape
	void addBox(uint64_t id, const glm::dvec3& dimensions, float mass = 0.f); // btBoxShape
	void addCylinder(uint64_t id, float radius,  float height, float mass = 0.f); // btCylinderShape
	void addCapsule(uint64_t id, float radius, float height, float mass = 0.f); // btCapsuleShape
	
	//void addEmpty(uint64_t id, float mass = 0.f); // btEmptyShape

	//void addDynamicMesh(uint64_t id, const std::string& file, float mass); // btConvexHull or btConvexPointCloudShape

	//void addStaticMesh(uint64_t id, const std::string& file); // btBvhTriangleMeshShape
	void addStaticPlane(uint64_t id); // btStaticPlaneShape
	//void addStaticHeightmap(uint64_t id, const std::string& file); // btHeightfieldTerrainShape

	void rayTest(const glm::dvec3& from, const glm::dvec3& to, std::vector<RayHit>& hits);
	RayHit rayTest(const glm::dvec3& from, const glm::dvec3& to);

	void sphereTest(float radius, const glm::dvec3& position, const glm::dquat& rotation, std::vector<SweepHit>& hits);
	void sphereSweep(uint64_t id, float radius, const glm::dvec3& fromPos, const glm::dquat& fromRot, const glm::dvec3& toPos, const glm::dvec3& toRot, std::vector<SweepHit>& hits);

	friend class Collider;
};