#include "GameState.h"

using namespace std;

void GameState::initGameState() {
	// Player Truck
	addEntity("player_truck1", PhysType::Vehicle, new Transform(), vector<string>{
		"assets/models/truck1/truck1.obj",
		"assets/models/tire1/tire1_front2.obj",
		"assets/models/tire1/tire1_front1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj"});

	// Landscape
	addEntity("landscape", PhysType::StaticMesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1.obj"});

	// Test Static Mesh
	addEntity("test_staticmesh", PhysType::StaticMesh, new Transform(vec3(0.f, -10.f, 25.f)), vector<string>{
		"assets/models/primitives/primitives_cube1.obj"});

	// Test RigidBody
	addEntity("test_rigidbody", PhysType::RigidBody, new Transform(), vector<string>{
		"assets/models/primitives/primitives_cube1.obj"});
}

void GameState::addEntity(string name, PhysType type, Transform* transform, vector<string> modelPaths) {
	// Create new entity at end of list
	entityList.emplace_back();
	// Name
	entityList.back().name = name;
	// Physics Type
	entityList.back().type = type;
	// Transform
	entityList.back().transform = transform;
	// Local Transforms
	for (int i = 0; i < modelPaths.size(); i++) {
		entityList.back().localTransforms.push_back(new Transform());
	}
	// Model
	entityList.back().model = new Model();
	for (int i = 0; i < modelPaths.size(); i++) {
		entityList.back().model->addMesh(modelPaths.at(i));
	}
}

Entity GameState::findEntity(string name) {
	for (int i = 0; i < entityList.size(); i++) {
		if (entityList.at(i).name == name) {
			return entityList.at(i);
		}
	}
	return Entity();
}