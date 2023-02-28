#include "AiController.h"

#include <vector>
#include <map>
using namespace std;


AiController::AiController(){
}


void AiController::initAiSystem(GameState* gameState) {
	this->gameState = gameState;
	this->state = 0; // Start state
	//this->aiVehicle = aiVehicle;
	
	
}




void AiController::StateController() {

	if (state == 0) {
		string currentTrailer = FindTrailer();
	}
	/*if (state == 1) {
		CollectTrailer();
	}
	if (state == 2) {
		DropOff();
	}*/


}

string AiController::FindTrailer() {


	for (Entity entity : gameState->entityList) {
		if (entity.name[0] == 't') {
			trailerLocations.push_back(make_pair(entity.name, entity.transform->getPosition()));
		}
	}


	string tempName;
	float distance = 10000000000.f;
	glm::vec3 aiPosition = gameState->findEntity("vehicle_1")->transform->getPosition();
	
	for (int i = 0; i < trailerLocations.size(); i++) {
		// dx^2 + dz ^2
		float tempDistanceX = (aiPosition.x * aiPosition.x) + (trailerLocations[i].second.x * trailerLocations[i].second.x);
		float tempDistanceY = (aiPosition.y * aiPosition.y) + (trailerLocations[i].second.y * trailerLocations[i].second.y);
		
		float tempDistance = tempDistanceX + tempDistanceY;
		//cout << tempDistance << endl;
		//cout << trailerLocations[i].second.x << " " << trailerLocations[i].second.y << " " << trailerLocations[i].second.z << endl;
	
		if (tempDistance < distance) {
			tempName = trailerLocations[i].first;
			distance = tempDistance;
		}

		
	}
	//cout << "trailer selected: " << tempName << endl;

	//state = 1;


	return tempName;


}

void AiController::CollectTrailer(string trailerName) {
	glm::vec3 trailerPosition = gameState->findEntity(trailerName)->transform->getPosition();
	// Drive to position
}


void AiController::DropOff() {

}

void AiController::BumpPlayer() {

}