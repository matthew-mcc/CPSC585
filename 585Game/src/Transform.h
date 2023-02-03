#pragma once
#include <glm.hpp>
#include <gtx/quaternion.hpp>

/*
All relevant information that the physics engine needs - can be updated later but position + rotation is enough to begin simulations.
*/
class Transform {


public:
	glm::vec3 position;
	glm::quat rotation;

	Transform() {
		this->position = glm::vec3();
		this->rotation = glm::quat();
	};


};