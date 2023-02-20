#pragma once
#include <Boilerplate/Window.h>
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include <vector>
#include <iostream>
#include "vehicle2/PxVehicleAPI.h"
#include "../snippetvehicle2common/enginedrivetrain/EngineDrivetrain.h"
#include "../snippetvehicle2common/serialization/BaseSerialization.h"
#include "../snippetvehicle2common/serialization/EngineDrivetrainSerialization.h"
#include "../snippetvehicle2common/SnippetVehicleHelpers.h"

#include <Boilerplate/Timer.h>
#include <Entity.h>
#include <GameState.h>

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle2;
using namespace glm;
using namespace std;

struct Vehicle {
	EngineDriveVehicle vehicle;
	vector<PxRigidDynamic*> attachedTrailers;
	vector<PxD6Joint*> attachedJoints;
};

class PhysicsSystem {

public:
	// Constructor
	PhysicsSystem(){};
	// Initializer
	void initPhysicsSystem(GameState* gameState);
	// Physics Update
	void stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, Timer* timer);

private:
	void initPhysX();
	void cleanupPhysX();
	void initPhysXMeshes();
	void initMaterialFrictionTable();
	void initVehicles(int vehicleCount);
	void spawnTrailer();
	void processTrailerCollision();
	Vehicle* getPullingVehicle(PxRigidDynamic* trailer);
	void attachTrailer(PxRigidDynamic* trailer, Vehicle* vehicle);
	void detachTrailer(PxRigidDynamic* trailer, Vehicle* vehicle);

	GameState* gameState;
};


class ContactReportCallback : public physx::PxSimulationEventCallback {
public:
	bool contactDetected = false;
	PxContactPairHeader contactPair;

private:
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
		PX_UNUSED(pairHeader);
		PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);

		contactPair = pairHeader;
		contactDetected = true;
	}
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
		const physx::PxTransform* poseBuffer,
		const physx::PxU32 count) {}

	//void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count){}

};