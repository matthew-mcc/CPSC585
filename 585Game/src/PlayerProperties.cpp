#include <PlayerProperties.h>


void PlayerProperties::setPlayerControlled() {
	playerControlled = true;
}

void PlayerProperties::setAiControlled() {
	playerControlled = false;
}

// Takes all player vehicle input from callbacks and updates player properties variables
void PlayerProperties::updateCallbacks(std::shared_ptr<CallbackInterface> callback_ptr) {

	brake = callback_ptr->brake;
	reverse = callback_ptr->reverse;
	throttle = callback_ptr->throttle;
	steer = callback_ptr->steer;

	AirPitch = callback_ptr->AirPitch;
	AirRoll = callback_ptr->AirRoll;

	boost = callback_ptr->boosterrrrr;

	addTrailer = callback_ptr->addTrailer;

	updateBoost();
}


void PlayerProperties::updateBoost() {



}















