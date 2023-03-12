#pragma once
#include <vector>
#include <Entity.h>
#include "AudioManager.h"

using namespace std;

class GameState {
public:
	// Public Functions
	GameState() {}
	void initGameState(AudioManager* audio);
	Entity* addEntity(string name, PhysType physType, DrawType drawType, Transform* transform, vector<string> modelPaths);
	Entity* findEntity(string name);
	Entity* spawnTrailer();
	Entity* spawnVehicle();
	void endGame();

	// Entity Tracking
	vector<Entity> entityList;
	Entity* winner;

	// Flags
	bool gameEnded = false;

	// Audio 
	AudioManager* audio_ptr = nullptr;
	glm::vec3 listener_position;
};