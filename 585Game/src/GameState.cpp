#include "GameState.h"
#include <stdlib.h>
#include <time.h> 

using namespace std;

int vehiclesSpawned = 0;
int trailersSpawned = 0;

void GameState::initGameState() {
	Entity* e;

	// Landscape
	e = addEntity("landscape", PhysType::StaticMesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1.obj"});

	// Center Oil Rig
	/*
	e = addEntity("oil_rig_center", PhysType::StaticMesh, new Transform(vec3(0.f, 0.f, 25.f)), vector<string>{
		"assets/models/oil_rig1/oil_rig1.obj"});
	*/

	// Center Portal
	e = addEntity("portal_center", PhysType::StaticMesh, new Transform(vec3(0.0f, 0.0f, 33.0f)), vector<string>{
		"assets/models/portal1/portal1_1.obj",
		"assets/models/portal1/portal1_2.obj",
		"assets/models/portal1/portal1_3.obj",
		"assets/models/portal1/portal1_4.obj"});

	// Center Platform
	e = addEntity("platform_center", PhysType::StaticMesh, new Transform(), vector<string>{
		"assets/models/platform1/platform1_1.obj",
		"assets/models/platform1/platform1_2.obj"});
	e = addEntity("platform_center_decal", PhysType::None, new Transform(), vector<string>{
		"assets/models/platform1/platform1_3.obj"});
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
	// Child Entity Counter
	entityList.back().nbChildEntities = 0;

	if (type == PhysType::Vehicle) {
		entityList.back().playerProperties = new PlayerProperties();
	}
	else {
		entityList.back().playerProperties = NULL;
	}

	// Return Pointer to Entity
	return &entityList.back();
}


Entity* GameState::findEntity(string name) {
	for (int i = 0; i < entityList.size(); i++) {
		if (entityList.at(i).name == name) {
			return &entityList.at(i);
		}
	}
	return new Entity();
}


Entity* GameState::spawnTrailer() {
	Entity* e;
	string name = "trailer_" + to_string(trailersSpawned);
	string modelPath;
	
	// Randomize trailer model
	switch (rand() % 3) {
	case 0:
		modelPath = "assets/models/trailer1/trailer1.obj";
		break;
	case 1:
		modelPath = "assets/models/trailer2/trailer2.obj";
		break;
	case 2:
		modelPath = "assets/models/trailer3/trailer3.obj";
		break;
	}

	e = addEntity(name, PhysType::RigidBody, new Transform(), vector<string>{
		modelPath,
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj"});
	e->localTransforms.at(1)->setPosition(vec3(-0.88f, -0.80f, -0.70f));
	e->localTransforms.at(2)->setPosition(vec3(0.88f, -0.80f, -0.70f));
	e->localTransforms.at(3)->setPosition(vec3(-0.88f, -0.80f, 0.70f));
	e->localTransforms.at(4)->setPosition(vec3(0.88f, -0.80f, 0.70f));
	trailersSpawned++;
	return e;
}


Entity* GameState::spawnVehicle() {
	string name = "vehicle_" + to_string(vehiclesSpawned);
	addEntity(name, PhysType::Vehicle, new Transform(), vector<string>{
		"assets/models/truck1/truck1.obj",
		"assets/models/tire1/tire1_front2.obj",
		"assets/models/tire1/tire1_front1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj"});
	vehiclesSpawned++;
}