#pragma once
#include <Boilerplate/Window.h>
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include <vector>
#include <iostream>
#include "vehicle2/PxVehicleAPI.h"

#include <Boilerplate/Timer.h>
#include <Boilerplate/Window.h>
#include <Entity.h>
#include <GameState.h>


class PhysicsSystem {

public:
	// Constructor
	PhysicsSystem();

	// Physics Update
	void stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer);
};