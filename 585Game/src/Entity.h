#pragma once

#include<string>
#include "Boilerplate/Model.h"
#include "Transform.h"

/*
This is similar to a GameObject in Unity - but much more simplified.

An entity will be composed of a name (identifier), a Model (what we are rendering) and a Transform (information for the physics engine).

*/
class Entity {
public:
	std::string name;
	Model* model;
	Transform* transform;
};