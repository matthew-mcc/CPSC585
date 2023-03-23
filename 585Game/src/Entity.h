#pragma once
#include <string>
#include "Boilerplate/Model.h"
#include "Transform.h"
#include "PlayerProperties.h"

using namespace std;

enum class PhysType {
	None,
	StaticMesh,
	Vehicle,
	Trailer,
	RigidBody
};

enum class DrawType {
	Mesh,
	Decal,
	Invisible
};

// Entity Class
// Contains individual object information
class Entity {
public:
	string name = "unnamed_entity";
	PhysType physType = PhysType::None;
	DrawType drawType = DrawType::Mesh;
	Transform* transform;
	vector<Transform*> localTransforms;
	Model* model;
	PlayerProperties* playerProperties;
	int nbChildEntities;
	bool onGround = false;
};