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
#include <PlayerProperties.h>

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle2;
using namespace glm;
using namespace std;

#define shape1 1
#define shape2 2
/*#define shape3 4
#define shape4 8
#define shape5 16
#define shape6 32*/


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
	// Changed to PlayerProperties
	//void stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, Timer* timer);
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
	bool AirOrNot = false;
	int gate = 0;
	vector<PxShape*>wheelshapes;
	vector<Vehicle*> cars;
private:
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
		//PX_UNUSED(pairHeader);
		//PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);
		/*if (pairHeader.pairs->events.isSet(PxPairFlag::eNOTIFY_TOUCH_FOUND)) {
			std::cout << "Collision Detected!" << std::endl;
			//AirOrNot = false;
		}*/
		if (pairHeader.pairs->events.isSet(PxPairFlag::eNOTIFY_TOUCH_LOST) 
			&& (cars.at(0)->vehicle.mPhysXState.physxActor.rigidBody == pairHeader.actors[0] ||
				cars.at(0)->vehicle.mPhysXState.physxActor.rigidBody == pairHeader.actors[1])) {
			AirOrNot = true;
			gate = 0;
		}
		contactPair = pairHeader;
		contactDetected = true;
	}
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
		if (pairs->status == PxPairFlag::eNOTIFY_TOUCH_FOUND && gate == 3) {
			AirOrNot = false;
			//cout << count << endl;
		}
		else {
			if (pairs->triggerShape == wheelshapes[0])
				gate |= shape1;
			else if (pairs->triggerShape == wheelshapes[1])
				gate |= shape2;
		}
	}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
		const physx::PxTransform* poseBuffer,
		const physx::PxU32 count) {}

	//void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count){}

};