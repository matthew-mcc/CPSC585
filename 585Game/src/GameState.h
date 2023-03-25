#pragma once
#include <vector>
#include <Entity.h>
#include "AudioManager.h"

using namespace std;

class GameState {
public:
	// Public Functions
	GameState() {}
	Entity* addEntity(string name, PhysType physType, DrawType drawType, Transform* transform, vector<string> modelPaths);
	Entity* findEntity(string name);
	Entity* spawnTrailer();
	Entity* spawnVehicle();
	void endGame();
	void resetGameState(AudioManager* audio);

	// Entity Tracking
	vector<Entity> entityList;
	Entity* winner;

	// Flags
	bool inMenu = true;
	bool gameEnded = false;

	// Game Parameters
	int numVehicles = 2;
	int numTrailers = 30;

	// Audio 
	AudioManager* audio_ptr = nullptr;
	glm::vec3 listener_position;

private:
	void initGameState();
};