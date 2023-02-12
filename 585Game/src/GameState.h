#pragma once
#include <vector>
#include <Entity.h>

using namespace std;

class GameState {
public:
	GameState() {}
	void initGameState();
	Entity* addEntity(string name, PhysType type, Transform* transform, vector<string> modelPaths);
	Entity findEntity(string name);

	// List of all game entities
	vector<Entity> entityList;
};