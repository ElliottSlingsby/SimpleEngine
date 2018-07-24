#include "Physics.hpp"

#include <glm/gtc/quaternion.hpp>

inline void fromBullet(const btVector3& from, glm::vec3* to) {
	to->x = from.x();
	to->y = from.y();
	to->z = from.z();
}

inline void fromBullet(const btQuaternion& const from, glm::quat* to) {
	to->w = from.w();
	to->x = from.x();
	to->y = from.y();
	to->z = from.z();
}

inline void toBullet(const glm::vec3& from, btVector3* to) {
	to->setX(from.x);
	to->setY(from.y);
	to->setZ(from.z);
}

inline void toBullet(const glm::quat& from, btQuaternion* to) {
	to->setW(from.w);
	to->setX(from.x);
	to->setY(from.y);
	to->setZ(from.z);
}

Physics::Physics(Engine & engine) : _engine(engine) {
	SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	SYSFUNC_ENABLE(SystemInterface, update, 0);
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

void Physics::initiate(const std::vector<std::string>& args) {
	_collisionConfiguration = new btDefaultCollisionConfiguration();
	_dispatcher = new btCollisionDispatcher(_collisionConfiguration);
	_overlappingPairCache = new btDbvtBroadphase();
	_solver = new btSequentialImpulseConstraintSolver;
	_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher, _overlappingPairCache, _solver, _collisionConfiguration);
}

void Physics::update(double dt) {

}

void Collider::getWorldTransform(btTransform & worldTrans) const{

}

void Collider::setWorldTransform(const btTransform & worldTrans){

}