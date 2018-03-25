#pragma once

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <LinearMath\btMotionState.h>
#include <LinearMath\btTransform.h>

#include "Collider.hpp"

/*
Add options for disabling certain trasnform options

like, e.g. disable everything but parenting only x and y components 

*/

struct Transform : public btMotionState {
	glm::dvec3 position;
	glm::quat rotation;
	glm::vec3 scale = { 1.f, 1.f, 1.f };

	Transform* parent = nullptr;
	std::vector<Transform*> children;

	glm::dvec3 worldPosition() const {
		glm::dvec3 rPosition = position;
		Transform* rParent = parent;

		while (rParent != nullptr) {
			rPosition = rParent->position + static_cast<glm::dquat>(rParent->rotation) * rPosition;
			rParent = rParent->parent;
		}

		return rPosition;
	}

	glm::quat worldRotation() const {
		glm::quat rRotation = rotation;
		Transform* rParent = parent;

		while (rParent != nullptr) {
			rRotation = rParent->rotation * rRotation;
			rParent = rParent->parent;
		}

		return rRotation;
	}

	glm::vec3 worldScale() const {
		glm::vec3 rScale = scale;
		Transform* rParent = parent;

		while (rParent != nullptr) {
			rScale *= rParent->scale;
			rParent = rParent->parent;
		}

		return rScale;
	}

	void setParent(Transform* other) {
		if (parent)
			removeParent();
		
		other->children.push_back(this);
		parent = other;
	}

	void setChild(Transform* other) {
		if (std::find(children.begin(), children.end(), other) != children.end())
			return;
		
		children.push_back(other);
		other->setParent(this);
	}

	void removeParent() {
		if (parent)
			parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
		
		parent = nullptr;
	}

	void removeChildren() {
		for (Transform* child : children)
			child->parent = nullptr;
		
		children.clear();
	}

	~Transform() {
		removeParent();
		removeChildren();
	}

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