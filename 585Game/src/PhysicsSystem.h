#pragma once
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include<vector>
#include <iostream>


class PhysicsSystem {

public:
	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
	
	//PhysX management Class Instances.

	// Storing all transforms in our simulation
	//std::vector<Transform> transformList;

	// PhysX stuff, don't care about this too much it shouldnt change.
	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback gErrorCallback;
	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;
	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;
	physx::PxMaterial* gMaterial = NULL;
	physx::PxPvd* gPvd = NULL;

	void updateTransforms();
	PhysicsSystem();
	physx::PxVec3 getPosition();

};