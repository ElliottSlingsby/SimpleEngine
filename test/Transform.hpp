#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

struct Transform : public btMotionState {
	glm::dvec3 position;
	glm::quat rotation;
	glm::vec3 scale = { 1.f, 1.f, 1.f };

	uint64_t parent = 0;

	void getWorldTransform(btTransform& transform) const override {
		transform.setOrigin(btVector3(
			static_cast<btScalar>(position.x), 
			static_cast<btScalar>(position.y), 
			static_cast<btScalar>(position.z)
		));

		transform.setRotation(btQuaternion(
			static_cast<btScalar>(rotation.x), 
			static_cast<btScalar>(rotation.y), 
			static_cast<btScalar>(rotation.z), 
			static_cast<btScalar>(rotation.w)
		));
	}

	void setWorldTransform(const btTransform& transform) override {
		position.x = static_cast<double>(transform.getOrigin().x());
		position.y = static_cast<double>(transform.getOrigin().y());
		position.z = static_cast<double>(transform.getOrigin().z());

		rotation.x = static_cast<float> (transform.getRotation().x());
		rotation.y = static_cast<float>(transform.getRotation().y());
		rotation.z = static_cast<float>(transform.getRotation().z());
		rotation.w = static_cast<float>(transform.getRotation().w());
	}
};