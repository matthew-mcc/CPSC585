#include "GameState.h"

using namespace std;

void GameState::initGameState() {
	// Player Truck
	addEntity("player_truck1", true, Transform(), vector<string>{
		"assets/models/test_truck1/test_truck1.obj"});

	// Landscape
	addEntity("landscape", false, Transform(), vector<string>{
		"assets/models/landscape1/landscape1_1.obj",
		"assets/models/landscape1/landscape1_2.obj",
		"assets/models/landscape1/landscape1_3.obj",
		"assets/models/landscape1/landscape1_4.obj"});
}

void GameState::addEntity(string name, bool bphysicsEntity, Transform transform, vector<string> modelPaths) {
	// Create new entity at end of list
	entityList.emplace_back();
	// Name
	entityList.back().name = name;
	// PhysicsEntity
	entityList.back().bphysicsEntity = bphysicsEntity;
	// Transform
	entityList.back().transform = &transform;
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
}