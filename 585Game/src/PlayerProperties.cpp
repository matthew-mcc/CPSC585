#include <PlayerProperties.h>
#include <Boilerplate/Timer.h>

void PlayerProperties::setPlayerControlled() {
	playerControlled = true;
}

void PlayerProperties::setAiControlled() {
	playerControlled = false;
}

void PlayerProperties::addScore(int toAdd) {
	playerScore = playerScore + toAdd;
}

int PlayerProperties::getScore() {
	return playerScore;
}

// Takes all player vehicle input from callbacks and updates player properties variables
void PlayerProperties::updateCallbacks(std::shared_ptr<CallbackInterface> callback_ptr) {

	brake = callback_ptr->brake;
	reverse = callback_ptr->reverse;
	throttle = callback_ptr->throttle;
	steer = callback_ptr->steer;

	AirPitch = callback_ptr->AirPitch;
	AirRoll = callback_ptr->AirRoll;

	addTrailer = callback_ptr->addTrailer;

	boost_status_cb = callback_ptr->boosterrrrr;

	if (callback_ptr->reset == 0.f) {
		reset = 0.f;
	}
	else {
		reset += callback_ptr->reset;
	}
	updateBoost();
}

// Boosting, how works? 
// We press boost, we start increasing speed, start consuming meter
// We stop pressing boost, after a period of time, our boost meter recovers 
// We have a ton of variables, but maybe not all will be used

void PlayerProperties::updateBoost() {
	// Get timer pointer
	Timer* timer = &Timer::Instance();

	// Determine if it's possible to boost
	if (boost_status_cb && boost_meter > 0.0f){
		boost_meter = boost_meter - boost_consumption_rate * timer->getDeltaTime();
		boost_status = true;
	}
	if (boost_meter < 0.0f) {
		boost_meter = 0.0f;
		boost_status = false;
	}
	if (!boost_status_cb) {
		boost_status = false;
	}

	// Case 1 = we start boosting
	if (boost_status) {
		boost = boost_strength;
		boost_meter = boost_meter - boost_consumption_rate * timer->getDeltaTime();
	}

	// Case 2 = we are not boosting, in recovery mode
	if(!boost_status) {
		boost = 0.0f;
		// For now no recovery time
		if (boost_meter < 100.0f) {
			boost_meter = boost_meter + boost_recovery_rate * timer->getDeltaTime();
		}
		if (boost_meter > 100.0f) {
			boost_meter = 100.0f;
		}
	}
}















