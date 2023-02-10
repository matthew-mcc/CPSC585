#pragma once
#include<string>
#include "Boilerplate/Model.h"
#include "Transform.h"

using namespace std;

/*
This is similar to a GameObject in Unity - but much more simplified.

An entity will be composed of a name (identifier), a Model (what we are rendering) and a Transform (information for the physics engine).

*/
class Entity {
public:
	string name = "unnamed_entity";
	bool bphysicsEntity = false; 
	bool isRigidBody = false;
	Transform* transform;
	vector<Transform*> localTransforms;
	Model* model;
};