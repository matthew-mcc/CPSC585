#pragma once
#include<string>
#include "Boilerplate/Model.h"
#include "Transform.h"

using namespace std;

enum class PhysType {
	None,
	StaticMesh,
	Vehicle,
	RigidBody
};

// Entity Class
// Contains individual object information
class Entity {
public:
	string name = "unnamed_entity";
	PhysType type = PhysType::None;
	Transform* transform;
	vector<Transform*> localTransforms;
	Model* model;
};