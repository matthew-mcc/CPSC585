#pragma once
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include<vector>
#include <iostream>
#include "vehicle2/PxVehicleAPI.h"

#include <Boilerplate/Timer.h>
#include "Entity.h"


class PhysicsSystem {

public:
	PhysicsSystem();
	void stepPhysics(std::vector<Entity> entityList, Timer* timer);

	//void updateTransforms(std::vector<Entity> entityList);
	//physx::PxVec3 getPosition();
	
	//PhysX management Class Instances.

	// Storing all transforms in our simulation
	//std::vector<Transform> transformList;

};