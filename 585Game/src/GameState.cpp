#include "GameState.h"
#include <stdlib.h>
#include <time.h> 

using namespace std;

int vehiclesSpawned = 0;
int trailersSpawned = 0;

void GameState::initGameState(AudioManager* audio) {
	audio_ptr = audio;
	Entity* e;

	// Landscape
	e = addEntity("landscape", PhysType::StaticMesh, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1.obj"});

	e = addEntity("landscape_detail", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape_detail1.obj"});

	e = addEntity("landscape_junk", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/junk1/junk1.obj"});

	// Sky Sphere
	e = addEntity("sky_sphere", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/sky_sphere1/sky_sphere1.obj"});

	// Map Border (Invisible Wall)
	e = addEntity("map_border", PhysType::StaticMesh, DrawType::Invisible, new Transform(), vector<string>{
		"assets/models/map_border1/map_border1.obj"});

	// Oil Rigs
	e = addEntity("oil_rigs", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/oil_rig1/oil_rig2.obj"});

	// Center Portal
	e = addEntity("portal_center", PhysType::StaticMesh, DrawType::Mesh, new Transform(vec3(0.0f, 0.0f, 32.0f)), vector<string>{
		"assets/models/portal1/portal1_1.obj",
		"assets/models/portal1/portal1_2.obj"});

	// Center Platform
	e = addEntity("platform_center", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/platform1/platform1.obj"});

	e = addEntity("decal_test", PhysType::None, DrawType::Decal, new Transform(), vector<string>{
		"assets/models/decals/decal1.obj"});
}

Entity* GameState::addEntity(string name, PhysType physType, DrawType drawType, Transform* transform, vector<string> modelPaths) {
	// Create new entity at end of list
	entityList.emplace_back();
	// Name
	entityList.back().name = name;
	// Physics Type
	entityList.back().physType = physType;
	// Draw Type
	entityList.back().drawType = drawType;
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

	if (physType == PhysType::Vehicle) {
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
	return NULL;
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

	e = addEntity(name, PhysType::Trailer, DrawType::Mesh, new Transform(), vector<string>{
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
	addEntity(name, PhysType::Vehicle, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/truck1/truck1.obj",
		"assets/models/tire1/tire1_front2.obj",
		"assets/models/tire1/tire1_front1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj"});
	vehiclesSpawned++;
}

void GameState::endGame() {
	// Give a default vehicle to compare against
	Entity* winningVehicle = findEntity("vehicle_0");

	// Get list of vehicles
	vector<Entity*> vehicles;
	for (int i = 0; i < vehiclesSpawned; i++) {
		string name = "vehicle_" + to_string(i);
		vehicles.push_back(findEntity(name));
	}

	// Determine vehicle with highest score
	for (int i = 0; i < vehicles.size(); i++) {
		if (vehicles.at(i)->playerProperties->getScore() > winningVehicle->playerProperties->getScore()) {
			winningVehicle = vehicles.at(i);
		}
	}
	// Check for ties
	for (int i = 0; i < vehicles.size(); i++) {
		if (winningVehicle->playerProperties->getScore() == vehicles.at(i)->playerProperties->getScore()) {
			if (winningVehicle != vehicles.at(i)) {
				winningVehicle = NULL;
				break;
			}
		}
	}
	winner = winningVehicle;
	gameEnded = true;
}