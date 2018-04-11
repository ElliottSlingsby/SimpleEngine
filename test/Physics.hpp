#pragma once

#include "Config.hpp"
#include "Collider.hpp"

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

	Collider* _addCollider(uint64_t id, btScalar mass, btCollisionShape* shape);

	void _createRigidBody(uint64_t id, btScalar mass, const btVector3& localInertia);

	void _recursiveGetMasses(uint64_t id, std::vector<btScalar>* masses);

	void _recursiveUpdateCompoundShape(uint64_t id, std::vector<btScalar>* masses);

	void _removeFromWorld(uint64_t id);

	void _setCenterOfMass(uint64_t id, const btVector3& position);

	void _recalculateCenterOfMass(uint64_t id);

	void _recalculateMass(uint64_t id);

	uint64_t _rootCollider(uint64_t id) const;

public:
	struct RayHit {
		uint64_t id = 0;
		Vec3 position;
		Vec3 normal;
	};

	struct SweepHit {
		uint64_t id = 0;
		Vec3 position;
	};

	Physics(Engine& engine);
	~Physics();

	void load(int argc, char** argv);
	void update(double dt);

	void updateWorldTransform(uint64_t id);
	void updateCompoundShape(uint64_t id);
	
	void setGravity(const Vec3& direction);
	
	Collider* addSphere(uint64_t id, float radius, float mass = 0.f); // btSphereShape
	Collider* addBox(uint64_t id, const Vec3& dimensions, float mass = 0.f); // btBoxShape
	Collider* addCylinder(uint64_t id, float radius,  float height, float mass = 0.f); // btCylinderShape
	Collider* addCapsule(uint64_t id, float radius, float height, float mass = 0.f); // btCapsuleShape
	
	//Collider* addEmpty(uint64_t id, float mass = 0.f); // btEmptyShape

	//Collider* addDynamicMesh(uint64_t id, const std::string& file, float mass); // btConvexHull or btConvexPointCloudShape

	//Collider* addStaticMesh(uint64_t id, const std::string& file); // btBvhTriangleMeshShape
	Collider* addStaticPlane(uint64_t id); // btStaticPlaneShape
	//Collider* addStaticHeightmap(uint64_t id, const std::string& file); // btHeightfieldTerrainShape

	void rayTest(const Vec3& from, const Vec3& to, std::vector<RayHit>& hits);
	RayHit rayTest(const Vec3& from, const Vec3& to);

	void sphereTest(float radius, const Vec3& position, const Quat& rotation, std::vector<SweepHit>& hits);
	void sphereSweep(uint64_t id, float radius, const Vec3& fromPos, const Quat& fromRot, const Vec3& toPos, const Vec3& toRot, std::vector<SweepHit>& hits);

	friend class Collider;
};