#pragma once
#include <vector>
#include <Entity.h>

using namespace std;

class GameState {
public:
	GameState() {}
	void initGameState();
	void addEntity(string name, bool bphysicsEntity, Transform transform, vector<string> modelPaths);

	// List of all game entities
	vector<Entity> entityList;

private:
	
};