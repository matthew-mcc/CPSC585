#pragma once
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "Model.h"
#include<vector>
#include <iostream>


class PhysicsSystem {

public:
	std::vector<physx::PxRigidDynamic*> rigidDynamicList;
	std::vector<Transform> transformList;
	std::vector<Model> modelList;
	//PhysX management Class Instances.

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