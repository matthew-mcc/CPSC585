#pragma once
#include "GameState.h"
#include<map>
#include<string>
class AiController {


	/*
		State Works As Follows:
		0 --> Find Trailer (start state)
		- This is not movement, this is just deciding which one to go to

		1 --> Collect Trailer
		- Once the closest trailer has been found, go get it!

		2 --> Drop Off
		- Once 5 trailers have been found, drop it off!

		3 --> Bump Player
		- Will be implemented last (no notes yet ;( )
	*/

public:

	int state;
	

	AiController();

	void initAiSystem(GameState* gameState);

	string FindTrailer();
	void CollectTrailer(string trailerName);
	void DropOff();
	void BumpPlayer();

	void StateController();

private:

	GameState* gameState;
	vector<pair<string, glm::vec3>> trailerLocations;
	Entity* aiVehicle;

};