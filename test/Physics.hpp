#pragma once

#include "Config.hpp"

#include <btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>

#include <string>
#include <unordered_map>

class Physics {
	Engine& _engine;

	// (copy pasted from bullet3 example code)

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration;

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;

	btSequentialImpulseConstraintSolver* solver;

	btDiscreteDynamicsWorld* dynamicsWorld;

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	

	// primitive (sphere, box, cylinder, capsule, cone, elipsoid)
	
	// convex hulls
	
	// static concave hulls

	//enum HullType {
	//	Primitive, // btSphereShape, btBoxShape, btCylinderShape, btCapsuleShape, btConeShape, btMultiSphereShape
	//	Dynamic, // btConvexPointCloudShape
	//	Static // btBvhTriangleMeshShape, btHeightfieldTerrainShape, btStaticPlaneShape
	//};


	std::vector<btCollisionShape*> shapes; // all shapes, primitives will be duplicated, convex hulls won't
	std::unordered_map<std::string, uint32_t> _filesShapes; // indexes to loaded convex hull shapes

public:
	enum Shapes {
		Sphere,
		Box
	};

	Physics(Engine& engine);
	~Physics();

	void load(int argc, char** argv);
	void update(double dt);

	void createRigidBody(uint64_t id, glm::vec3 size, float mass);
	
	//void setGravity(glm::vec3 direction);
	//
	//void addSphere(float radius, float mass = 0.f, glm::vec3 origin = glm::vec3());
	//void addBox(glm::vec3 dimensions, float mass = 0.f, glm::vec3 origin = glm::vec3());
	//void addCylinder(float radius,  float height, float mass = 0.f, glm::vec3 origin = glm::vec3());
	//void addCapsule(float radius, float height, float mass = 0.f, glm::vec3 origin = glm::vec3());
	//void addEllipsoid(glm::vec3 dimensions, float mass = 0.f, glm::vec3 origin = glm::vec3());
	//
	//void addDynamicMesh(const std::string& file, float mass, glm::vec3 origin = glm::vec3());
	//
	//void addStaticMesh(const std::string& file, glm::vec3 origin = glm::vec3());
	//void addStaticPlane(glm::vec3 origin = glm::vec3());
	//void addStaticHeightmap(const std::string& file, glm::vec3 origin = glm::vec3());
};