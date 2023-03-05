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

	// debug 
	bool addTrailer = false;

private:
	bool playerControlled = false;
	int playerScore = 0;

	bool boost_status = false;
	bool boost_status_cb = false;

	bool boost_recovery = false;

	float boost_reset_time = 1.0f;
	float boost_reset_countdown = 0.0f;

	float boost_speed_limit = 0.2f;
	float boost_increase_rate = 0.05f;				// How much boost is added when holding down boost button
	float boost_consumption_rate = 0.25f;			// How much meter is consumed when holding down boost button
	float boost_recovery_rate = 0.3f;				// How fast meter recovers when not holding down boost button
};










