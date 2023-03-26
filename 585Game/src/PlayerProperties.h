#pragma once
#include <Boilerplate/Window.h>

class PlayerProperties {
public:
	void setPlayerControlled();
	void setAiControlled();
	void addScore(int toAdd);
	int getScore();
	void updateCallbacks (std::shared_ptr<CallbackInterface> callback_ptr);

	void updateBoost();

	// Controls
	float throttle = 0.f;
	float brake = 0.f;
	float reverse = 0.f;
	float steer = 0.f;
	float AirPitch = 0.f;
	float AirRoll = 0.f;

	// Boost
	float boost = 0.f;
	float boost_meter = 100.0f;
	float boost_max_velocity = 60.f;

	// Reset
	float reset = 0.f;
	float reset_max = 3.0f;

	// debug 
	bool addTrailer = false;

	// Stolen Trailers
	int nbStolenTrailers = 0;

private:
	bool playerControlled = false;
	int playerScore = 0;

	bool boost_status = false;
	bool boost_status_cb = false;

	bool boost_recovery = false;

	float boost_reset_time = 1.0f;
	float boost_reset_countdown = 0.0f;

	float boost_strength = 20.0f;					// Strength of boost when activated
	float boost_consumption_rate = 25.0f;			// How much meter is consumed when holding down boost button
	float boost_recovery_rate = 15.0f;				// How fast meter recovers when not holding down boost button
};










