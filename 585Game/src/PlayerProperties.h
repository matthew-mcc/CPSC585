#pragma once
#include <Boilerplate/Window.h>



class PlayerProperties {
public:
	// Ok, I'm a little confused as to how this interacts with callbacks
	// I think this class has a callback, then increases brake accordingly
	// Provides finer control over inputs
	// I don't know how this will interact with xbox controls
	// What will we want?
	// An update function that takes in inputs for sure
	// Some constructors

	void setPlayerControlled();
	void setAiControlled();
	void updateCallbacks (std::shared_ptr<CallbackInterface> callback_ptr);

	void updateBoost();

	// Forgo some get functions for now, lets make sure it's up to date
//	float getBrake();




	bool playerControlled = false;
	int playerScore = 0;



	float brake = 0.f;
	float reverse = 0.f;
	float throttle = 0.f;
	float steer = 0.f;
	float AirPitch = 0.f;
	float AirRoll = 0.f;
	float boost = 0.f;
	float boost_bar = 100.0f;

	bool addTrailer = false;

private:
	// What various bullshit will we want for boost
	float boost_reset_time = 2.0f;
	float boost_recovery_rate = 5.0f;
	float boost_consumption_rate = 2.0f;


};










