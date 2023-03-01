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
		selectedTrailer = FindTrailer();
		//state = 1;
	}
	if (state == 1) {
		CollectTrailer();

		// if # trailers > 5, state --> 2, else state --> 0

	}

	/*if (state == 2) {
		DropOff();
	}*/


}

pair<string, glm::vec3> AiController::FindTrailer() {


	for (Entity entity : gameState->entityList) {
		if (entity.name[0] == 't') {
			trailerLocations.push_back(make_pair(entity.name, entity.transform->getPosition()));
		}
	}


	string tempName;
	glm::vec3 tempVec;
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
			tempVec = trailerLocations[i].second;
			distance = tempDistance;
			
		}

		
	}
	


	return make_pair(tempName, tempVec);


}

void AiController::CollectTrailer() {
	/*glm::vec3 trailerPosition = gameState->findEntity(trailerName)->transform->getPosition();*/
	// Drive to position

	

	//Somehow we need to pass this trailerPosition into physics system
}


void AiController::DropOff() {

}

void AiController::BumpPlayer() {

}