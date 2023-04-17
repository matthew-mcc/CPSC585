#include "GameState.h"
#include <stdlib.h>
#include <time.h> 

using namespace std;

// Counters
int vehiclesSpawned = 0;
int trailersSpawned = 0;

void GameState::initGameState() {
	Entity* e;

	// Landscape
	e = addEntity("landscape", PhysType::StaticMesh, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1.obj"});

	e = addEntity("landscape_background", PhysType::StaticMesh, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape1_background1.obj"});

	e = addEntity("landscape_detail", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/landscape1/landscape_detail1.obj"});

	e = addEntity("landscape_junk", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/junk1/junk1.obj"});

	e = addEntity("rock_net1", PhysType::StaticMesh, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/rock_net1/rock_net1.obj"});

	e = addEntity("rock_net2", PhysType::None, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/rock_net1/rock_net2.obj"});

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
		"assets/models/platform1/platform1_1.obj",
		"assets/models/platform1/platform1_2.obj"});
	e = addEntity("platform_collision", PhysType::StaticMesh, DrawType::Invisible, new Transform(), vector<string>{
		"assets/models/platform1/platform1_collision.obj"});

	// Portal Effect
	//e = addEntity("effect_portal", PhysType::None, DrawType::Decal, new Transform(), vector<string>{
		//"assets/models/effects/effect_portal1.obj"});

	// Decals
	e = addEntity("decal_tracks", PhysType::None, DrawType::Decal, new Transform(), vector<string>{
		"assets/models/decals/decal_tracks1.obj"});
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
	Entity* vehicle = addEntity(name, PhysType::Vehicle, DrawType::Mesh, new Transform(), vector<string>{
		"assets/models/truck1/truck1.obj",
		"assets/models/truck1/truck1_fan1.obj",
		"assets/models/tire1/tire1_front2.obj",
		"assets/models/tire1/tire1_front1.obj",
		"assets/models/tire1/tire1_back2.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back1.obj",
		"assets/models/tire1/tire1_back2.obj"});
	vehicle->localTransforms.at(1)->setPosition(vec3(0.f, 1.2879f, -0.82054f));
	vehiclesSpawned++;
}

int GameState::calculatePoints(int vehicleIndex, int totalTrailers, int stolenTrailers) {
	string name = "vehicle_" + to_string(vehicleIndex);
	int score = 0;
	for (int i = 0; i < totalTrailers; i++) {
		score += 3;
	}
	for (int i = 0; i < stolenTrailers; i++) {
		score += 1;
	}
	return score;

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

void GameState::resetGameState(AudioManager* audio) {
	audio_ptr = audio;
	entityList.clear();
	winner = NULL;
	vehiclesSpawned = 0;
	trailersSpawned = 0;
	initGameState();
}

void GameState::menuEventHandler(vector<std::shared_ptr<CallbackInterface>> cbps) {
	// Only handle events when in menu
	if (inMenu) {
		// Party Player Increment / Decrement
		if (showPlayerSelect) {
			// Increase Player Count
			if (cbps[0]->navigateR && playerSelectIndex < 4) {
				playerSelectIndex++;
				cbps[0]->navigateR = false;
				audio_ptr->MenuClick(1, listener_position);
			}
			// Decrease Player Count (or show back option)
			else if (cbps[0]->navigateL && playerSelectIndex > 1) {
				playerSelectIndex--;
				cbps[0]->navigateL = false;
				audio_ptr->MenuClick(1, listener_position);
			}
		}
		// Menu Navigation
		else {
			// Navigate Right
			if (cbps[0]->navigateR && !showInfo) {
				menuOptionIndex = (menuOptionIndex + 1) % nbMenuOptions;
				cbps[0]->navigateR = false;
				audio_ptr->MenuClick(1, listener_position);
			}
			// Navigate Left
			else if (cbps[0]->navigateL && !showInfo) {
				menuOptionIndex = (menuOptionIndex - 1) % nbMenuOptions;
				cbps[0]->navigateL = false;
				audio_ptr->MenuClick(1, listener_position);
			}
			// Navigation Wrap-Around
			if (menuOptionIndex < 0) {
				menuOptionIndex = nbMenuOptions - 1;
			}
		}

		// Buttons
		if (cbps[0]->menuConfirm) {
		// SUB-MENUS
			// Info Back
			if (showInfo) {
				showInfo = false;
				audio_ptr->MenuClick(0, listener_position);
			}
			// Player Select Confirm Options
			else if (showPlayerSelect) {
				// Player Select Back
				if (playerSelectIndex == 1) {
					showPlayerSelect = false;
					audio_ptr->MenuClick(0, listener_position);
				}
				// Party Play
				else {
					numPlayers = playerSelectIndex;
					loading = true;
					audio_ptr->MenuClick(2, listener_position);
				}
			}
		// MAIN OPTIONS
			// Solo
			else if (menuOptionIndex == 0) {
				numPlayers = 1;
				loading = true;
				audio_ptr->MenuClick(2, listener_position);
			}
			// Party
			else if (menuOptionIndex == 1) {
				playerSelectIndex = 2;
				showPlayerSelect = true;
				audio_ptr->MenuClick(0, listener_position);
			}
			// Info
			else if (menuOptionIndex == 2) {
				showInfo = true;
				audio_ptr->MenuClick(0, listener_position);
			}
			// Quit
			else if (menuOptionIndex == 3) {
				quit = true;
			}
			cbps[0]->menuConfirm = false;
		}
	}
}


void GameState::ingameMenuEventHandler(vector<std::shared_ptr<CallbackInterface>> cbps) {
	// Toggled when in a game and player 1 hits escape
	if (inGameMenu) {
		// Resume
		if (ingameOptionIndex == 0) {
			if (cbps[0]->navigateD) {
				cbps[0]->navigateD = false;
				ingameOptionIndex++;
			}
			if (cbps[0]->menuConfirm) {
				audio_ptr->MenuClick(0, listener_position);
				cbps[0]->ingameMenu = false;
				inGameMenu = false;
			}
		}

		// Volume - SFX
		if (ingameOptionIndex == 1) {
			if (cbps[0]->navigateU) {
				cbps[0]->navigateU = false;
				ingameOptionIndex--;
			}
			if (cbps[0]->navigateD) {
				cbps[0]->navigateD = false;
				ingameOptionIndex++;
			}
			// Increasing volume
			if (cbps[0]->navigateR) {
				sfxChange = true;
				cbps[0]->navigateR = false;
				if (audio_ptr->SFXVolume < audio_ptr->maxVolume) {
					audio_ptr->SFXVolume += 0.2f;
				}
			}
			// Decreasing volume
			if (cbps[0]->navigateL) {
				sfxChange = true;
				cbps[0]->navigateL = false;
				if (audio_ptr->SFXVolume > 0.0f) {
					if (audio_ptr->SFXVolume < 0.1f) {
						audio_ptr->SFXVolume = 0.0f;	// If this isn't here, it might not be at exactly 0, leading to some sound
					}
					audio_ptr->SFXVolume -= 0.2f;
				}
			}
		}

		// Volume - Music
		if (ingameOptionIndex == 2) {
			if (cbps[0]->navigateU) {
				cbps[0]->navigateU = false;
				ingameOptionIndex--;
			}
			if (cbps[0]->navigateD) {
				cbps[0]->navigateD = false;
				ingameOptionIndex++;
			}
			// Increasing music volume
			if (cbps[0]->navigateR) {
				musicChange = true;
				cbps[0]->navigateR = false;
				if (audio_ptr->musicVolume < audio_ptr->maxVolume) {
					audio_ptr->musicVolume = audio_ptr->musicVolume + 0.2f;
				}
			}
			// Decreasing music volume
			if (cbps[0]->navigateL) {
				musicChange = true;
				cbps[0]->navigateL = false;
				if (audio_ptr->musicVolume > 0.0f) {
					if (audio_ptr->musicVolume < 0.1f) {
						audio_ptr->musicVolume = 0.0f;
					}
					else {
						audio_ptr->musicVolume -= 0.2f;
					}
				}
			}
		}


		// Quit Button
		if (ingameOptionIndex == 3) {
			if (cbps[0]->navigateU) {
				cbps[0]->navigateU = false;
				ingameOptionIndex--;
			}

			// Help me, Peter!
			if (cbps[0]->menuConfirm) {
				cbps[0]->menuConfirm = false;
				audio_ptr->MenuClick(0, listener_position);
				cbps[0]->ingameMenu = false;
				gameEnded = false;
				inGameMenu = false;
				inMenu = true;
			}
		}

		// Apply changes to volume
		if (sfxChange) {
			std::cout << "SFX: " << to_string(audio_ptr->SFXVolume) << std::endl;
			sfxChange = false;
			audio_ptr->setVolume(audio_ptr->e_dropoff, audio_ptr->SFXVolume);
			audio_ptr->setVolume("vehicle_0_engine", audio_ptr->playerEngineVolume * audio_ptr->SFXVolume);
			audio_ptr->setVolume("vehicle_0_tire", audio_ptr->playerTireVolume * audio_ptr->SFXVolume);
			audio_ptr->setVolume("vehicle_0_honk", audio_ptr->honkVolume * audio_ptr->SFXVolume);

			for (int i = 1; i <numVehicles; i++) {
				std::string vehicleName = "vehicle_";
				vehicleName += to_string(i);
				audio_ptr->setVolume(vehicleName + "_engine", audio_ptr->npcEngineVolume * audio_ptr->SFXVolume);
				audio_ptr->setVolume(vehicleName + "_tire", audio_ptr->npcTireVolume * audio_ptr->SFXVolume);
				audio_ptr->setVolume(vehicleName + "_honk", audio_ptr->honkVolume * audio_ptr->SFXVolume);
			}
		}

		if (musicChange) {
			std::cout << "Music: " << to_string(audio_ptr->musicVolume) << std::endl;
			musicChange = false;
			audio_ptr->setVolume("SpaceIntro", audio_ptr->musicVolume);
			audio_ptr->setVolume("SpaceMusic2", audio_ptr->musicVolume);
		}
	}
}