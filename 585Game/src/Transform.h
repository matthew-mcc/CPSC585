#pragma once
#include <glm.hpp>
#include <gtx/quaternion.hpp>

using namespace glm;

// Transform Information for Game Entities
class Transform {


public:
// CONSTRUCTOR
	Transform(vec3 pos = vec3(), quat rot = quat()) {
		this->position = pos;
		this->rotation = rot;
		this->linearVelocity = vec3();
		update();
	};

// GETTERS
	vec3 getPosition() {
		return this->position;
	}

	quat getRotation() {
		return this->rotation;
	}

	vec3 getLinearVelocity() {
		return this->linearVelocity;
	}

	vec3 getForwardVector() {
		return this->forwardVector;
	}

	vec3 getUpVector() {
		return this->upVector;
	}

	vec3 getRightVector() {
		return this->rightVector;
	}

	bool getOnGround() {
		return this->onGround;
	}

// SETTERS
	void setPosition(vec3 pos) {
		this->position = pos;
		update();
	}

	void setRotation(quat rot) {
		this->rotation = rot;
		update();
	}

	void setLinearVelocity(vec3 vel) {
		this->linearVelocity = vel;
		update();
	}

	void setOnGround(bool onGround) {
		this->onGround = onGround;
	}

private:
	// Transform
	vec3 position;
	quat rotation;

	// Velocity
	vec3 linearVelocity;

	// Orientation
	vec3 forwardVector;
	vec3 upVector;
	vec3 rightVector;

	// State
	bool onGround = false;

	// Update Function
	// Updates vectors based on position and rotation
	void update() {
		mat4 m = toMat4(this->rotation);
		this->forwardVector = (vec3)(m * vec4(0.f, 0.f, 1.f, 0.f));
		this->upVector = (vec3)(m * vec4(0.f, 1.f, 0.f, 0.f));
		this->rightVector = (vec3)(m * vec4(-1.f, 0.f, 0.f, 0.f));
	}
};