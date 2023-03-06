#pragma once
#include <vector>
#include <Entity.h>

using namespace std;

class GameState {
public:
	// Public Functions
	GameState() {}
	void initGameState();
	Entity* addEntity(string name, PhysType type, Transform* transform, vector<string> modelPaths);
	Entity* findEntity(string name);
	Entity* spawnTrailer();
	Entity* spawnVehicle();
	void endGame();

	// Entity Tracking
	vector<Entity> entityList;
	Entity* winner;

	// Flags
	bool gameEnded = false;
};