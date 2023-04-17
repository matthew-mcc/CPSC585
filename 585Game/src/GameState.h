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
	int calculatePoints(int vehicleIndex, int totalTrailers, int stolenTrailers);
	void endGame();
	void resetGameState(AudioManager* audio);
	void menuEventHandler(vector<std::shared_ptr<CallbackInterface>> cbps);
	void ingameMenuEventHandler(vector<std::shared_ptr<CallbackInterface>> cbps);

	// Entity Tracking
	vector<Entity> entityList;
	Entity* winner;

	// Flags
	bool inMenu = true;
	bool loading = false;
	bool quit = false;
	bool showInfo = false;
	bool showPlayerSelect = false;
	bool gameEnded = false;
	int playerSelectIndex = 2;
	int menuOptionIndex = 0;
	int nbMenuOptions = 4;

	bool inGameMenu = false;
	int ingameOptionIndex = 0;
	int nbIngameOptions = 4;
	bool sfxChange = false;
	bool musicChange = false;

	// Game Parameters
	int numPlayers = 1;
	int numVehicles = 6;
	int numTrailers = 30;

	// Audio 
	AudioManager* audio_ptr = nullptr;
	glm::vec3 listener_position;

private:
	void initGameState();
};