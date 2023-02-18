#include "GameState.h"

using namespace std;

void GameState::initGameState() {
	Entity* e;

	// Landscape
	addEntity("landscape", PhysType::StaticMesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1.obj"});

	// Center Oil Rig
	addEntity("oil_rig_center", PhysType::StaticMesh, new Transform(vec3(0.f, 0.f, 25.f)), vector<string>{
		"assets/models/oil_rig1/oil_rig1.obj"});
}


Entity* GameState::addEntity(string name, PhysType type, Transform* transform, vector<string> modelPaths) {
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

	return &entityList.back();
}


Entity GameState::findEntity(string name) {
	for (int i = 0; i < entityList.size(); i++) {
		if (entityList.at(i).name == name) {
			return entityList.at(i);
		}
	}
	return Entity();
}


Entity* GameState::spawnTrailer() {
	Entity* e;
	e = addEntity("trailer", PhysType::RigidBody, new Transform(), vector<string>{
		"assets/models/trailer1/trailer1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj"});
	e->localTransforms.at(1)->setPosition(vec3(-0.88f, -0.80f, -0.70f));
	e->localTransforms.at(2)->setPosition(vec3(0.88f, -0.80f, -0.70f));
	e->localTransforms.at(3)->setPosition(vec3(-0.88f, -0.80f, 0.70f));
	e->localTransforms.at(4)->setPosition(vec3(0.88f, -0.80f, 0.70f));
	return e;
}


Entity* GameState::spawnVehicle() {
	addEntity("player_truck1", PhysType::Vehicle, new Transform(), vector<string>{
		"assets/models/truck1/truck1.obj",
		"assets/models/tire1/tire1_front2.obj",
		"assets/models/tire1/tire1_front1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj"});
}