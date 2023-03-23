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
#include "AiController.h"

#include "Boilerplate/AudioEngine.h"


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

struct Trailer;
struct Vehicle;

struct Trailer {
	PxRigidDynamic* rigidBody;
	Vehicle* vaccuumTarget;
	int entityIndex;
	bool isFlying = false;
	bool isTowed = false;
	float groundDistance = 0.0f;
};

struct Vehicle {
	EngineDriveVehicle vehicle;
	vector<Trailer*> attachedTrailers;
	vector<PxD6Joint*> attachedJoints;
	bool onGround = false;
	int AI_State;
	int AI_CurrTrailerIndex;
	
	// Maybe we don't need a map of values, but instead just one personality?
	/*map<string, float> AI_Personalities = {
		{"Aggressive", 0.f},
		{"Defensive", 0.f},
		{"Optimal", 0.f},
		{"Reckless", 0.f}
	};*/
	string AI_Personality = "Default";
	/*
	Potential Personalities:
	1. Aggressive --> Tries ramming into other players often, and will relentlessly chase players.
	Might even go for the boxes that other players are trying to get (guess which box they are going for and get it).
	2. Defensive / Longest Train --> Tries getting at least 10 trailers before dropoff. Will avoid other players on the map
	to ensure that it can maintain a long train.
	3. Optimal --> Does a mixture of stealing and drop off that is the "best" way to play the game. Should be the best AI.
	4. Reckless --> Prioritizes going very fast and flying around the map. Will likely be low scoring but fun to watch.
	*/
};

class PhysicsSystem {

public:
	// Constructor
	PhysicsSystem(){};
	// Initializers
	void initPhysX();
	void initPhysicsSystem(GameState* gameState, AiController* aiController);
	// Physics Update
	// Changed to PlayerProperties
	//void stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, Timer* timer);
	void stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, Timer* timer);
	
	static glm::vec3 CameraRaycasting( glm::vec3 camposd,float side,float down_back );
	static bool CameraIntercetionRaycasting(glm::vec3 campos);

private:
	void cleanupPhysX();
	void initPhysXMeshes();
	void initMaterialFrictionTable();
	void initVehicles(int vehicleCount);
	PxVec3 randomSpawnPosition();
	void spawnTrailer();
	void processTrailerCollision();
	void updateJointLimits(Vehicle* vehicle);
	void changeRigidDynamicShape(PxRigidDynamic* rigidBody, PxBoxGeometry newGeom);
	Trailer* getTrailerObject(PxRigidDynamic* trailerBody);
	Vehicle* getVehicleObject(PxRigidDynamic* vehicleBody);
	int getVehicleIndex(Vehicle* vehicle);
	Vehicle* getPullingVehicle(Trailer* trailer);
	void attachTrailer(Trailer* trailer, Vehicle* vehicle);
	void detachTrailer(Trailer* trailer, Vehicle* vehicle, Vehicle* vaccuumTarget);
	void dropOffTrailer(Vehicle* vehicle);
	void resetCollectedTrailers();
	void trailerForces(float deltaTime);
	GameState* gameState;
	AiController* aiController;
	int spawnedTrailers = 0;


	// AI
	int AI_State;
	int currTrailerIndex;
	void AI_InitSystem();
	void AI_MoveTo(Vehicle* vehicle, PxVec3 destination);
	void AI_FindTrailer(Vehicle* vehicle);
	void AI_CollectTrailer(Vehicle* vehicle, PxReal timestep);
	void AI_DropOff(Vehicle* vehicle);
	void AI_BumpPlayer(Vehicle* vehicle);
	void AI_StateController(Vehicle* vehicle, PxReal timestep);
	void AI_DetermineAttackPatterns(Vehicle* vehicle, Vehicle* target);
	void AI_DefensiveManeuvers(Vehicle* self, Vehicle* attacker, PxReal timestep);
	void AI_ApplyBoost(Vehicle* vehicle);
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
		}
		if (pairHeader.pairs->events.isSet(PxPairFlag::eNOTIFY_TOUCH_LOST) 
			&& (cars.at(0)->vehicle.mPhysXState.physxActor.rigidBody == pairHeader.actors[0] ||
				cars.at(0)->vehicle.mPhysXState.physxActor.rigidBody == pairHeader.actors[1])) {
			AirOrNot = true;
			gate = 0;
		}*/
		contactPair = pairHeader;
		if(pairHeader.pairs->events.isSet(PxPairFlag::eNOTIFY_TOUCH_FOUND))
			contactDetected = true;
	}
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
		/*if (pairs->status == PxPairFlag::eNOTIFY_TOUCH_FOUND && gate == 3) {
			AirOrNot = false;
			//cout << count << endl;
		}
		else {
			if (pairs->triggerShape == wheelshapes[0])
				gate |= shape1;
			else if (pairs->triggerShape == wheelshapes[1])
				gate |= shape2;
		}*/
	}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
		const physx::PxTransform* poseBuffer,
		const physx::PxU32 count) {}

	//void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count){}

};