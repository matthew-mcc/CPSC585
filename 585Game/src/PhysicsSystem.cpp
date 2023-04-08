#define _USE_MATH_DEFINES

#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include "../snippetcommon/SnippetPVD.h"
#include "PhysicsSystem.h"
#include "Boilerplate/OBJ_Loader.h"

#include "Pathfinder.h"
#include "AiController.h"
#include "NavMesh.h"
#include <stdlib.h>
#include <time.h> 
#include <map>
#include <math.h>
#include<random>

//PhysX management class instances.
PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxMaterial* gMaterial = NULL;
PxMaterial* trailerMat = NULL;
PxPvd* gPvd = NULL;
ContactReportCallback* gContactReportCallback;

// AI globals
Pathfinder* pathfinder;
AiController* aiController;

// Raycasts
PxRaycastBuffer AircontrolBuffer;
PxRaycastBuffer TrailerBuffer;

// Vehicles
vector<Vehicle*> vehicles;
vector<PxVec3> vehicleStartPositions = vector<PxVec3>{
	PxVec3(0.0f, 8.0f, -245.0f),
	PxVec3(-250.0f, 8.0f, 0.0f),
	PxVec3(0.0f, 8.0f, 250.0f),
	PxVec3(250.0f, 8.0f, 0.0f),
	PxVec3(150.0f, 8.0f, 0.0f),
	PxVec3(50.0f, 8.0f, 0.0f)};
vector<PxQuat> vehicleStartRotations = vector<PxQuat>{
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f)};

// Trailers
vector<Trailer*> trailers;
float circle = 0.f;
int rigidBodyAddIndex;	// DEBUG ONLY
PxBoxGeometry attachedTrailerShape = PxBoxGeometry(.5f, .5f, .5f);
PxBoxGeometry detachedTrailerShape = PxBoxGeometry(1.2f, 1.2f, 1.2f);

// Joint Angle Limits
PxJointAngularLimitPair normalTwistJoint = PxJointAngularLimitPair(-0.2f, 0.2f);			// Pitch
PxJointLimitPyramid normalPyramidJoint = PxJointLimitPyramid(-0.4f, 0.4f, -0.01f, 0.01f);	// Yaw, Roll
PxJointAngularLimitPair tailTwistJoint = PxJointAngularLimitPair(-0.01f, 0.01f);			// Pitch
PxJointLimitPyramid tailPyramidJoint = PxJointLimitPyramid(-0.05f, 0.05f, -0.01f, 0.01f);	// Yaw, Roll


// Cooking
PxCooking* gCooking = NULL;
PxCookingParams* gParams;

//The path to the vehicle json files to be loaded.
const char* gVehicleDataPath = NULL;
int FrameCounter = 0;

//Vehicle simulation needs a simulation context
//to store global parameters of the simulation such as 
//gravitational acceleration.
PxVehiclePhysXSimulationContext gVehicleSimulationContext;

//Gravitational acceleration
const PxVec3 gGravity(0.0f, -9.81f, 0.0f);

//The mapping between PxMaterial and friction.
PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
PxU32 gNbPhysXMaterialFrictions = 0;
PxReal gPhysXDefaultMaterialFriction = 1.0f;

//Give the vehicle a name so it can be identified in PVD.
const char gVehicleName[] = "engineDrive";


vec3 toGLMVec3(PxVec3 pxTransform) {
	vec3 vector = vec3();
	vector.x = pxTransform.x;
	vector.y = pxTransform.y;
	vector.z = pxTransform.z;
	return vector;
}

quat toGLMQuat(PxQuat pxTransform) {
	quat quaternion = quat();
	quaternion.x = pxTransform.x;
	quaternion.y = pxTransform.y;
	quaternion.z = pxTransform.z;
	quaternion.w = pxTransform.w;
	return quaternion;
}

vector<int> getStolenTrailerIndices(Vehicle* vehicle) {
	vector<int> indices;
	for (int i = 0; i < vehicle->attachedTrailers.size(); i++) {
		if (vehicle->attachedTrailers.at(i)->isStolen) indices.push_back(i);
	}
	return indices;
}

PxVec3 PhysicsSystem::randomSpawnPosition() {
	// Iterate until desirable location is found
	while (true) {
		// radius = distance from circle center
		// If smaller than 45.0, try again (otherwise location will be inside portal)
		float radius = 250.f * sqrt(((float)rand()) / (RAND_MAX));
		if (radius < 45.f) continue;

		// angle = rotation around circle
		float angle = ((float)rand() / (RAND_MAX)) * 2 * M_PI;

		// x = x coordinate of point in circle
		// y = y coordinate of point in circle (offset by 32.0 due to portal location)
		float x = 0.0f + radius * cos(angle);
		float z = 32.f + radius * sin(angle);

		// Return final location
		return PxVec3(x, 60.f, z);
	}
}

int PhysicsSystem::getVehicleIndex(Vehicle* vehicle) {
	for (int i = 0; i < vehicles.size(); i++) {
		if (vehicles.at(i) == vehicle) {
			return i;
		}
	}
	return 0;
}

Vehicle* PhysicsSystem::getPullingVehicle(Trailer* trailer) {
	for (int i = 0; i < vehicles.size(); i++) {
		for (int j = 0; j < vehicles.at(i)->attachedTrailers.size(); j++) {
			if (trailer == vehicles.at(i)->attachedTrailers.at(j)) {
				return vehicles.at(i);
			}
		}
	}
	return NULL;
}

Trailer* PhysicsSystem::getTrailerObject(PxRigidDynamic* trailerBody) {
	for (int i = 0; i < trailers.size(); i++) {
		if (trailers.at(i)->rigidBody == trailerBody) {
			return trailers.at(i);
		}
	}
	return NULL;
}

Vehicle* PhysicsSystem::getVehicleObject(PxRigidDynamic* vehicleBody) {
	for (int i = 0; i < vehicles.size(); i++) {
		if (vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody == vehicleBody) {
			return vehicles.at(i);
		}
	}
	return NULL;
}

void PhysicsSystem::updateJointLimits(Vehicle* vehicle) {
	for (int i = 0; i < vehicle->attachedJoints.size(); i++) {
		if (i == vehicle->attachedJoints.size() - 1) {
			vehicle->attachedJoints.at(i)->setTwistLimit(tailTwistJoint);
			vehicle->attachedJoints.at(i)->setPyramidSwingLimit(tailPyramidJoint);
		}
		else {
			vehicle->attachedJoints.at(i)->setTwistLimit(normalTwistJoint);
			vehicle->attachedJoints.at(i)->setPyramidSwingLimit(normalPyramidJoint);
		}
	}
}

void PhysicsSystem::changeRigidDynamicShape(PxRigidDynamic* rigidBody, PxBoxGeometry newGeom) {
	PxShape* oldShape = NULL;
	rigidBody->getShapes(&oldShape, 1, 0);
	rigidBody->detachShape(*oldShape);

	PxShape* newShape = gPhysics->createShape(newGeom, *trailerMat);
	PxFilterData shapeFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
	newShape->setSimulationFilterData(shapeFilter);
	rigidBody->attachShape(*newShape);
}

void PhysicsSystem::processTrailerCollision() {
	// Reset contact flag, store trailer (1st in contact pair)
	gContactReportCallback->contactDetected = false;
	PxRigidDynamic* trailerBody = (PxRigidDynamic*)gContactReportCallback->contactPair.actors[0];
	Trailer* trailer = getTrailerObject(trailerBody);

	// Only process if the trailer is not flying (a.k.a being sucked into the portal)
	if (trailer == nullptr) return;
	if (!trailer->isFlying) {
		// Find the colliding vehicle
		for (int i = 0; i < vehicles.size(); i++) {
			if (vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody == gContactReportCallback->contactPair.actors[1]) {
				// Find vehicle that is pulling the trailer (if one exists)
				// If the pulling vehicle and colliding vehicle are different, detach from old vehicle, then attach to the colliding vehicle
				Vehicle* pullingVehicle = getPullingVehicle(trailer);
				if (pullingVehicle != NULL && pullingVehicle != vehicles.at(i)) {
					detachTrailer(trailer, pullingVehicle, vehicles.at(i));
					attachTrailer(trailer, vehicles.at(i));
					return;
				}
				// Otherwise if no pulling vehicle exists, simply attach trailer to colliding vehicle
				else if (pullingVehicle != vehicles.at(i)){
					attachTrailer(trailer, vehicles.at(i));
					return;
				}
			}
		}
	}
}

void PhysicsSystem::spawnTrailer() {
	// Create trailer entity
	gameState->spawnTrailer();
	PxVec3 spawnPos = randomSpawnPosition();

	// Create trailer shape
	PxShape* shape = gPhysics->createShape(detachedTrailerShape, *trailerMat);
	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
	shape->setSimulationFilterData(boxFilter);

	// Create trailer rigid dynamic
	PxRigidDynamic* trailerBody;
	trailerBody = gPhysics->createRigidDynamic(PxTransform(spawnPos));
	trailerBody->attachShape(*shape);
	trailerBody->setLinearDamping(0.5);
	trailerBody->setAngularDamping(1);
	trailerBody->setMass(2);
	gScene->addActor(*trailerBody);
	
	// Create trailer object
	Trailer* trailer = new Trailer();
	trailer->rigidBody = trailerBody;
	trailer->entityIndex = spawnedTrailers;
	trailers.push_back(trailer);
	spawnedTrailers++;
}

void PhysicsSystem::attachTrailer(Trailer* trailer, Vehicle* vehicle) {
	// Define local vars
	PxD6Joint* joint;
	PxTransform jointTransform;
	PxTransform trailerOffset;

	PxRigidBody* parentBody;

	// Modify trailer rigidbody parameters to appropriate towing values
	trailer->rigidBody->setLinearDamping(10);
	trailer->rigidBody->setAngularDamping(5);
	changeRigidDynamicShape(trailer->rigidBody, attachedTrailerShape);
	 
	// First Joint, set parent body to vehicle
	if (vehicle->attachedTrailers.size() == 0) {
		trailerOffset = PxTransform(PxVec3(0.0f, 0.5f, -3.f));
		parentBody = vehicle->vehicle.mPhysXState.physxActor.rigidBody;
	}

	// Not first joint, set parent body to last trailer in chain
	else {
		trailerOffset = PxTransform(PxVec3(0.0f, 0.0f, -3.f));
		parentBody = vehicle->attachedTrailers.back()->rigidBody;
	}

	// Update trailer's global position to be in-line of the trail
	trailer->rigidBody->setGlobalPose(parentBody->getGlobalPose().transform(trailerOffset));

	// Create joint
	jointTransform = PxTransform(trailer->rigidBody->getGlobalPose().p);
	joint = PxD6JointCreate(*gPhysics, parentBody, parentBody->getGlobalPose().getInverse().transform(jointTransform), trailer->rigidBody, trailer->rigidBody->getGlobalPose().getInverse().transform(jointTransform));
	if (joint != nullptr) {
		joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);

		// Add new trailer to attachedTrailers tracking list, update flags
		trailer->isTowed = true;
		trailer->vaccuumTarget = NULL;
		vehicle->attachedTrailers.push_back(trailer);
		vehicle->attachedJoints.push_back(joint);
		updateJointLimits(vehicle);

		// Play some audio
		glm::vec3 vehiclePos = toGLMVec3(trailer->rigidBody->getGlobalPose().p);
		//glm::vec3 playerPos = gameState->findEntity("vehicle_0")->transform->getPosition();

		//vehiclePos = vehiclePos - gameState->listener_position;

		//std::cout << "position: " << vehiclePos.x;
		//std::cout << ", " << vehiclePos.y;
		//std::cout << ", " << vehiclePos.z << std::endl;

		gameState->audio_ptr->Latch(vehiclePos);
	}
}

void PhysicsSystem::detachTrailer(Trailer* trailer, Vehicle* vehicle, Vehicle* vaccuumTarget) {
	// Init local vars
	int breakPoint = 0;
	vector<PxD6Joint*> newJoints;
	vector<Trailer*> newTrailers;

	// Loop through attached trailers until collided trailer is found, store index in breakPoint
	for (int i = 0; i < vehicle->attachedTrailers.size(); i++) {
		if (vehicle->attachedTrailers.at(i) == trailer) {
			breakPoint = i;
			break;
		}
		else {
			newJoints.push_back(vehicle->attachedJoints.at(i));
			newTrailers.push_back(vehicle->attachedTrailers.at(i));
		}
	}
	// Loop backwards through attached trailers, removing the back trailer iteratively until breakPoint is reached
	for (int i = vehicle->attachedTrailers.size() - 1; i >= breakPoint; i--) {
		vehicle->attachedJoints.at(i)->release();
		vehicle->attachedTrailers.at(i)->isTowed = false;
		if (vaccuumTarget == NULL) vehicle->attachedTrailers.at(i)->isStolen = false;
		else vehicle->attachedTrailers.at(i)->isStolen = true;
		vehicle->attachedTrailers.at(i)->vaccuumTarget = vaccuumTarget;
		vehicle->attachedTrailers.at(i)->rigidBody->setAngularDamping(1);
		vehicle->attachedTrailers.at(i)->rigidBody->setLinearDamping(1);
		changeRigidDynamicShape(vehicle->attachedTrailers.at(i)->rigidBody, detachedTrailerShape);
	}

	// Set new joint and trailer arrays for vehicle
	vehicle->attachedJoints = newJoints;
	vehicle->attachedTrailers = newTrailers;
	updateJointLimits(vehicle);
}

//PxRaycastHit hitBuffer[10];

bool PhysicsSystem::CameraIntercetionRaycasting(glm::vec3 campos) {
	PxRaycastBuffer cameraRayBuffer(0, 0);
	PxVec3 cam = PxVec3(campos.x, campos.y, campos.z); //where the ray shoots, the origin 
	PxVec3 start = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	//PxTransform vehicle_trans = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
	PxVec3 dir = start - cam; // we need the ray shoot from vehicle to camera, which then can detect the ground mesh. direction should base on global map
	//PxVec3 moving_vector(0.f);
	//PxVec3 Normal_ray_dir = cam - start;
	//PxVec3 Normal_shoot_origin;
	return (gScene->raycast(cam, dir.getNormalized(), dir.magnitude(), cameraRayBuffer, PxHitFlag::eMESH_BOTH_SIDES) && cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND);
		//moving_vector = dir * (cameraRayBuffer.block.distance / dir.magnitude());
		//Normal_shoot_origin = cam + dir * ((cameraRayBuffer.block.distance + 0.1f) / dir.magnitude());
		//moving_vector = vehicle_trans.rotateInv(moving_vector).getNormalized();
		//if (gScene->raycast(Normal_shoot_origin, Normal_ray_dir.getNormalized(), dir.magnitude(), cameraRayBuffer, PxHitFlag::eNORMAL) &&
		//	cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND)
		//	moving_vector += cameraRayBuffer.block.normal.getNormalized();
		//moving_vector = vehicle_trans.rotateInv(moving_vector);
		//return glm::vec3(moving_vector.x, moving_vector.y, moving_vector.z);
	//}
	//return glm::vec3(moving_vector.x, moving_vector.y, moving_vector.z);
}
glm::vec3 PhysicsSystem::CameraRaycasting(glm::vec3 campos,float side,float down_back, int playerNo) {
	PxRaycastBuffer cameraRayBuffer(0, 0);
	PxVec3 cam = PxVec3(campos.x, campos.y, campos.z); //where the ray shoots, the origin 
	PxVec3 start = vehicles.at(playerNo)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxTransform vehicle_trans = vehicles.at(playerNo)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose();
	PxVec3 dir =  start-cam; // we need the ray shoot from vehicle to camera, which then can detect the ground mesh. direction should base on global map
	PxVec3 backward = cam-start;
	backward.y = 0.f;
	PxVec3 downward = vehicle_trans.rotate(PxVec3(0.f, -1.f, 0.f));
	PxVec3 moving_vector(0.f);
	
	//prevent touch the ground  // just check different basis direction of the camera, give it no chance to touch the ground 
	//prevent backward touch  
	if (gScene->raycast(cam, backward.getNormalized(), down_back, cameraRayBuffer, PxHitFlag::eNORMAL) &&
		cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) {
		moving_vector += cameraRayBuffer.block.normal.getNormalized();
		
	}
	//prevent downward touch  
	if (gScene->raycast(cam, downward, down_back, cameraRayBuffer, PxHitFlag::eNORMAL) &&
		cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) {
		moving_vector += cameraRayBuffer.block.normal.getNormalized();
		
	}
	//prevent side touches  
	if (gScene->raycast(cam, downward.cross(backward).getNormalized(), side, cameraRayBuffer, PxHitFlag::eNORMAL) &&
		cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) {
		moving_vector += cameraRayBuffer.block.normal.getNormalized();
		
	}
	else if (gScene->raycast(cam, backward.cross(downward).getNormalized(), side, cameraRayBuffer, PxHitFlag::eNORMAL) &&
		cameraRayBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) {
		moving_vector += cameraRayBuffer.block.normal.getNormalized();
	}
	moving_vector = vehicle_trans.rotateInv(moving_vector);
	return glm::vec3(moving_vector.x, moving_vector.y, moving_vector.z);
	//----------------------------------------------------------------------------//
	//if get into the ground, get out ... 
	//Do we still want this enable? Like, the camera now will be hard to get blocked by ground, it's still possible,but you need to intend to do that
	//It generally happens while you trying to do a U turn on the map border with high speed, and the camera will be throw out of the ground
	//And even if you did, camera will reset to where it should be after all...
	//----------------------------------------------------------------------------//
	
}

void PhysicsSystem::trailerForces(float deltaTime) {
	for (int i = 0; i < trailers.size(); i++) {
		// PORTAL SPINNING FORCE
		if (trailers.at(i)->isFlying) {
			PxVec3 dir = (PxVec3(0.0f, 0.0f, 32.0f) - trailers.at(i)->rigidBody->getGlobalPose().p);
			dir = 3.0f * (dir / dir.magnitude());
			dir = PxVec3(dir.x, 0.0f, dir.z);

			// change the cof smaller will make the assemble process faseter. For example 10.f will looks like trailers immediately assemble to a point
			// make cof bigger will slower down the process, so we can have a better spin animation of trailers assemble to a point
			float cof = 40.0f;
			float height = clamp(((trailers.at(i)->rigidBody->getGlobalPose().p.y) / cof), 0.25f, 100.f); // start from 0.25f, which makes 1/0.25f be 4.f

			// the pulling force for trailers that assemble them to the central point
			dir = PxVec3(dir.x, 0.f, dir.z);

			// the coefficient for spin force, this should be initially big and grow slowly. So assign it to 
			float coffi = clamp((1.f / height), 0.1f, 4.f); // this will be 4.f at beginning

			// the coefficient for pulling force, this should be initially small and grow fast. So assign it to a log function
			float coffi2 = clamp(log2(height + 2.f), 1.f, 100.f); // this will be 1.f at beginning

			// therefore, eventually we want the pulling force defeat spin force, which let them assemble to a point
			// but we need to make the process reasonable slow that play can feel trailers are gradually pulling together rather than immediately.
			trailers.at(i)->rigidBody->addForce(dir * coffi2 * deltaTime * 100.f, PxForceMode().eVELOCITY_CHANGE);
			trailers.at(i)->rigidBody->addForce(PxVec3(-sinf(glm::radians(circle + i * 55)) * coffi, 0.5f, cosf(glm::radians(circle + i * 55)) * coffi) * deltaTime * 100.f, PxForceMode().eVELOCITY_CHANGE);
		}

		// STOLEN TRAILER VACCUUM FORCE
		else if (trailers.at(i)->vaccuumTarget != NULL) {
			PxVec3 target = trailers.at(i)->vaccuumTarget->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
			PxVec3 position = trailers.at(i)->rigidBody->getGlobalPose().p;
			trailers.at(i)->rigidBody->addForce((target - position).getNormalized() * deltaTime * 250.0f, PxForceMode().eVELOCITY_CHANGE);
		}
	}
}

void PhysicsSystem::dropOffTrailer(Vehicle* vehicle) {
	vector<PxD6Joint*> newJoints;
	vector<Trailer*> newTrailers;
	int nbTrailers = vehicle->attachedTrailers.size();
	int nbStolenTrailers = 0;

	// Add trailers to flyTrailer list
	// Release all joints attached to vehicle
	for (int i = 0; i < nbTrailers; i++) {
		if (vehicle->attachedTrailers.at(i)->isStolen) {
			vehicle->attachedTrailers.at(i)->isStolen = false;
			nbStolenTrailers++;
		}
		vehicle->attachedTrailers.at(i)->isFlying = true;
		vehicle->attachedTrailers.at(i)->isTowed = false;
		changeRigidDynamicShape(vehicle->attachedTrailers.at(i)->rigidBody, detachedTrailerShape);
		vehicle->attachedJoints.at(i)->release();
	}

	// Reset vehicle's joint and trailer lists
	vehicle->attachedJoints = newJoints;
	vehicle->attachedTrailers = newTrailers;
	updateJointLimits(vehicle);

	// Find entity of vehicle that triggered the dropoff
	// Add score to this entity
	Entity* vehicleEntity = gameState->findEntity("vehicle_" + to_string(getVehicleIndex(vehicle)));
	int scoreToAdd = gameState->calculatePoints(getVehicleIndex(vehicle), nbTrailers, nbStolenTrailers);
	vehicleEntity->playerProperties->addScore(scoreToAdd);
	cout << vehicleEntity->name << "'s new score: " << vehicleEntity->playerProperties->getScore() << ". Number Stolen: " << nbStolenTrailers << "\n";		// For debugging
}

void PhysicsSystem::resetCollectedTrailers() {
	for (int i = 0; i < trailers.size(); i++) {
		// Only concerned with currently flying trailers
		if (trailers.at(i)->isFlying) {
			// If trailer reaches a certain height, reset its position and flags
			if (trailers.at(i)->rigidBody->getGlobalPose().p.y > 40.0f) {
				trailers.at(i)->rigidBody->setGlobalPose(PxTransform(randomSpawnPosition()));
				trailers.at(i)->rigidBody->setLinearDamping(0.5);
				trailers.at(i)->rigidBody->setAngularDamping(1);
				trailers.at(i)->isFlying = false;
			}
		}
	}
}

void PhysicsSystem::initPhysX() {
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);


	PxInitExtensions(*gPhysics, gPvd);
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = gGravity;

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader;

	// Not sure if we need this
	gContactReportCallback = new ContactReportCallback();
	sceneDesc.simulationEventCallback = gContactReportCallback;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient){
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	trailerMat = gPhysics->createMaterial(0.f, 0.f, 0.f);

	PxInitVehicleExtension(*gFoundation);

	// Cooking init
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
	if (!gCooking)
		cout << "PxCreateCooking Failed!" << endl;
	gParams = new PxCookingParams(PxTolerancesScale());
	gParams->meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	gParams->meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	gCooking->setParams(*gParams);
}

void PhysicsSystem::cleanupPhysX() {
	PxCloseVehicleExtension();

	PX_RELEASE(gMaterial);
	PX_RELEASE(trailerMat);
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();
	}
	PX_RELEASE(gFoundation);
}

void PhysicsSystem::initPhysXMeshes() {

	// Loop through every entity, check if it's Physics Type is StaticMesh
	for (int i = 0; i < gameState->entityList.size(); i++) {
		if (gameState->entityList.at(i).physType == PhysType::StaticMesh) {

			// Loop through every mesh tied to the entity
			for (int j = 0; j < gameState->entityList.at(i).model->modelPaths.size(); j++) {
				// Load the .obj from a stored path
				objl::Loader loader;
				string objPath = gameState->entityList.at(i).model->modelPaths.at(j);
				loader.LoadFile(objPath);

				// Populate vertex and index arrays
				vector<PxVec3> vertexArr;
				vector<PxU32> indicesArr;
				for (int k = 0; k < loader.LoadedMeshes[0].Vertices.size(); k++) {
					float x = loader.LoadedMeshes[0].Vertices[k].Position.X + gameState->entityList.at(i).transform->getPosition().x + gameState->entityList.at(i).localTransforms.at(j)->getPosition().x;
					float y = loader.LoadedMeshes[0].Vertices[k].Position.Y + gameState->entityList.at(i).transform->getPosition().y + gameState->entityList.at(i).localTransforms.at(j)->getPosition().y;
					float z = loader.LoadedMeshes[0].Vertices[k].Position.Z + gameState->entityList.at(i).transform->getPosition().z + gameState->entityList.at(i).localTransforms.at(j)->getPosition().z;
					vertexArr.push_back(PxVec3(x, y, z));
				}
				for (int k = 0; k < loader.LoadedMeshes[0].Indices.size(); k++) {
					indicesArr.push_back(loader.LoadedMeshes[0].Indices[k]);
				}

				// Mesh Description for Triangle Mesh
				PxTriangleMeshDesc meshDesc;
				meshDesc.setToDefault();                                                                                                                                                            

				meshDesc.points.count = (PxU32)vertexArr.size();
				meshDesc.points.stride = sizeof(PxVec3);
				meshDesc.points.data = vertexArr.data();

				meshDesc.triangles.count = (PxU32)(indicesArr.size() / 3);
				meshDesc.triangles.stride = 3 * sizeof(PxU32);
				meshDesc.triangles.data = indicesArr.data();

				// Cook triangle mesh
				PxDefaultMemoryOutputStream writeBuffer;
				bool status = gCooking->cookTriangleMesh(meshDesc, writeBuffer);
				if (!status) cout << "Mesh Creation Failed!" << endl;

				// Cook mesh and store pointer
				PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				PxTriangleMesh* mesh = gPhysics->createTriangleMesh(readBuffer);

				PxTriangleMeshGeometry meshGeo = PxTriangleMeshGeometry(mesh, PxMeshScale(1));
				PxShape* meshShape = gPhysics->createShape(meshGeo, *gMaterial, true);
				
				if (gameState->entityList.at(i).name != "map_border") {
					PxFilterData meshFilter(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
					meshShape->setSimulationFilterData(meshFilter);
				}

				PxTransform meshTrans(physx::PxVec3(0, 0, 0), PxQuat(PxIdentity));
				PxRigidStatic* meshStatic = gPhysics->createRigidStatic(meshTrans);

				meshStatic->attachShape(*meshShape);
				gScene->addActor(*meshStatic);
			}
		}
	}
}

void PhysicsSystem::initMaterialFrictionTable() {
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	gPhysXMaterialFrictions[0].friction = 3.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 3.0f;
	gNbPhysXMaterialFrictions = 1;
}

int randNum(int low, int high) {
	random_device seeder;
	mt19937 engine(seeder());
	uniform_int_distribution<int> dist(low, high);
	int num = dist(engine);
	return num;
}

void PhysicsSystem::initVehicles(int vehicleCount) {
	srand(time(NULL));


	for (int i = 0; i < vehicleCount; i++) {
		// Create a new vehicle entity and physics struct
		gameState->spawnVehicle();
		vehicles.push_back(new Vehicle());

		// Init AI_State
		vehicles.at(i)->AI_State = 0;
		vehicles.at(i)->AI_CurrTrailerIndex = 0;
		vehicles.at(i)->AI_BoostMeter = 100.f;

		if (i == 1) {
			vehicles.at(i)->AI_Personality = "Defensive";
		}
		if (i == 2) {
			vehicles.at(i)->AI_Personality == "Aggressive";
		}
		if (i == 3) {
			vehicles.at(i)->AI_Personality == "Aggressive";
		}
		
		// Init AI Dropoff Threshold
		int threshold; // DO I need to init this value?

		if (vehicles.at(i)->AI_Personality == "Defensive") {
			threshold = randNum(6, 9);
		}
		else {
			threshold = randNum(3, 6);
		}
		vehicles.at(i)->AI_DropOffThreshold = threshold;
		


		//Load the params from json or set directly.
		gVehicleDataPath = "assets/vehicledata";
		readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", vehicles.back()->vehicle.mBaseParams);
		setPhysXIntegrationParams(vehicles.back()->vehicle.mBaseParams.axleDescription, gPhysXMaterialFrictions, gNbPhysXMaterialFrictions, gPhysXDefaultMaterialFriction, vehicles.back()->vehicle.mPhysXParams);
		readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json", vehicles.back()->vehicle.mEngineDriveParams);

		//Set the states to default.
		!vehicles.back()->vehicle.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE);

		//Apply a start pose to the physx actor and add it to the physx scene.
		PxTransform pose(vehicleStartPositions.at(i), vehicleStartRotations.at(i));
		vehicles.back()->vehicle.setUpActor(*gScene, pose, gVehicleName);

		//Set the vehicle in 1st gear.
		vehicles.back()->vehicle.mEngineDriveState.gearboxState.currentGear = vehicles.back()->vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
		vehicles.back()->vehicle.mEngineDriveState.gearboxState.targetGear = vehicles.back()->vehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
		vehicles.back()->vehicle.mEngineDriveParams.gearBoxParams.switchTime = 0.1;
		

		//Set the vehicle to use the automatic gearbox.
		vehicles.back()->vehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

		//Set up the simulation context.
		//The snippet is set up with
		//a) z as the longitudinal axis
		//b) x as the lateral axis 
		//c) y as the vertical axis.
		//d) metres  as the lengthscale.
		gVehicleSimulationContext.setToDefault();
		gVehicleSimulationContext.frame.lngAxis = PxVehicleAxes::ePosZ;
		gVehicleSimulationContext.frame.latAxis = PxVehicleAxes::ePosX;
		gVehicleSimulationContext.frame.vrtAxis = PxVehicleAxes::ePosY;
		gVehicleSimulationContext.scale.scale = 1.0f;
		gVehicleSimulationContext.gravity = gGravity;
		gVehicleSimulationContext.physxScene = gScene;
		gVehicleSimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;



		// Vehicle flags
		PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
		PxFilterData wheelFilter(COLLISION_FLAG_WHEEL, 0, 0, 0);


		//float halfLen = 0.5f;
		//PxMaterial *tem_m = gPhysics->createMaterial(0.f, 0.f, 0.f);
		//PxShape* sha = gPhysics->createShape(PxSphereGeometry(0.37f),*tem_m);
		//sha->setSimulationFilterData(wheelFilter);
		//sha->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		//sha->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);//set a trigger, for reset AirOrNot condition, it doesn't take part during physics simulation
		//gVehicle.mPhysXState.physxActor.rigidBody->attachShape(*sha); //not really work as expected
		PxU32 shapes = vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
		PxBoxGeometry chassisShape = PxBoxGeometry(0.9f, 0.5f, 2.5f);

		if (i == 0){// only simulate for player
			for (PxU32 j = 0; j < shapes; j++) {
				PxShape* shape = NULL;
				vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, j);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				if (j == 0) {
					//shape->setContactOffset(0.f);
					shape->setSimulationFilterData(vehicleFilter);
					shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
					shape->setGeometry(chassisShape);
				}
				/*else if (j == 1 || j == 6) {
					shape->setContactOffset(0.03f);
					shape->setSimulationFilterData(wheelFilter);
					shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
					gContactReportCallback->wheelshapes.push_back(shape);
				}*/
			}
			vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->setMaxAngularVelocity(4);
		}
		else { //AI cars 
			for (PxU32 j = 0; j < 1; j++) {
				PxShape* shape = NULL;
				vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, j);
				shape->setSimulationFilterData(vehicleFilter);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
				if (j == 0) {
					shape->setGeometry(chassisShape);
				}
			}
		}
			//shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}
	gContactReportCallback->cars = vehicles;
	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);


	
	
}

void PhysicsSystem::initPhysicsSystem(GameState* gameState, AiController* aiController) {
	this->gameState = gameState;
	this->aiController = aiController;
	vehicles.clear();
	trailers.clear();
	srand(time(NULL));
	initPhysXMeshes();
	initMaterialFrictionTable();
	initVehicles(gameState->numVehicles);

	// Baseline of 30 Trailers
	for (int i = 0; i < gameState->numTrailers; i++) {
		spawnTrailer();
	}
}

void PhysicsSystem::stepPhysics(vector<shared_ptr<CallbackInterface>> callback_ptrs, Timer* timer) {
	PxReal timestep;
	if (timer->getDeltaTime() > 0.1) {	// Safety check: If deltaTime gets too large, default it to (1 / 60)
		timestep = (1 / 60.f);
	}
	else {	// Otherwise set as deltaTime as normal
		timestep = (float)timer->getDeltaTime();
	}

	// Check for trailer contact reports
	// If detected, attached trailer to truck
	if (gContactReportCallback->contactDetected) {
		processTrailerCollision();
	}

	// Debug - Add trailer on key command
	if (callback_ptrs[0]->addTrailer) {
		//attachTrailer(trailers.at(rigidBodyAddIndex), vehicles.at(0));
		callback_ptrs[0]->addTrailer = false;
		rigidBodyAddIndex++;

		// Position Debug
		//std::cout << "position: " << gameState->findEntity("vehicle_0")->transform->getPosition().x;
		//std::cout << ", " << gameState->findEntity("vehicle_0")->transform->getPosition().y;
		//std::cout << ", " << gameState->findEntity("vehicle_0")->transform->getPosition().z << std::endl;

		//std::cout << "listener: " << gameState->listener_position.x;
		//std::cout << ", " << gameState->listener_position.y;
		//std::cout << ", " << gameState->listener_position.z << std::endl;
		//std::cout << "====================================" << std::endl;

		//std::cout << "boost: " << gameState->findEntity("vehicle_0")->playerProperties->boost << std::endl;
	

		// Stolen Trailer Debug
		//std::cout << "Stolen List: ";
		//for (int i = 0; i < gameState->findEntity("vehicle_0")->playerProperties->stolenTrailerIndices.size(); i++) {
		//	std::cout << gameState->findEntity("vehicle_0")->playerProperties->stolenTrailerIndices[i] << ", ";
		//}
		//std::cout << std::endl;

		std::cout << "engine: " << vehicles.at(0)->vehicle.mEngineDriveState.engineState.rotationSpeed << std::endl;

	
	}



	// Apply trailer forces
		// Spinning motion for dropped off trailers
		// Vaccuum effect for stolen trailers
	trailerForces(timestep);
	if (circle > 360) circle = 0;
	else circle++;

	// Reset collected trailers that have risen to a tall enough height
	resetCollectedTrailers();

	// Store entity list
	// Update player callbacks
	auto entityList = gameState->entityList;
	vector<Entity*> players;
	for (int i = 0; i < gameState->numPlayers; i++) {
		players.push_back(gameState->findEntity("vehicle_" + to_string(i)));
		players[i]->playerProperties->updateCallbacks(callback_ptrs[i]);
	}

	// Loop through each vehicle, update input and step physics simulation
	for (int i = 0; i < vehicles.size(); i++) {
		//Forward integrate the vehicle by a single timestep.
		//Apply substepping at low forward speed to improve simulation fidelity.
		const PxVec3 linVel = vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
		const PxVec3 forwardDir = vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
		const PxReal forwardSpeed = linVel.dot(forwardDir);
		PxTransform vehicle_transform = vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().getNormalized();
		const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
		vehicles.at(i)->vehicle.mComponentSequence.setSubsteps(vehicles.at(i)->vehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
		vehicles.at(i)->vehicle.step(timestep, gVehicleSimulationContext);

		// Trailer dropoff detection
		// (x - circle_x)^2 + (z - circle_z)^2 <= r^2
		// Where:
			// r = 30.0
			// circle_x = 0.0
			// circle_z = 32.0
		if (vehicle_transform.p.x * vehicle_transform.p.x + (vehicle_transform.p.z - 32.f) * (vehicle_transform.p.z - 32.f) <= 900.f){
			if (vehicles.at(i)->attachedTrailers.size() > 0) {
				dropOffTrailer(vehicles.at(i));

				std::string otherstr = "vehicle_";
				otherstr += to_string(i);

				// AUDIO : Dropoff Sound
				gameState->audio_ptr->Dropoff();
				
			}
		}

		// PLAYER VEHICLE INPUT
		if (i < gameState->numPlayers) {
			// On Ground
			if (gScene->raycast(vehicle_transform.p, vehicle_transform.rotate(PxVec3(0.f, -1.f, 0.f)), 0.7f, AircontrolBuffer) 
				&& AircontrolBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND){
				
				// Update ground flag
				if (vehicles.at(i)->onGround == false) vehicles.at(i)->landed = true;
				vehicles.at(i)->onGround = true;

				// Forward Drive
				if ((forwardSpeed > -1.0f && players[i]->playerProperties->throttle > players[i]->playerProperties->brake) ||
					forwardSpeed > 0.5f) {
					// Give vehicle a help in getting moving by instantly putting into 1st when close to stationary
					if (forwardSpeed < 0.5) {
						vehicles.at(i)->vehicle.mEngineDriveState.gearboxState.currentGear = 2;
					}
					// Set inputs for forward driving
					vehicles.at(i)->vehicle.mCommandState.throttle = players[i]->playerProperties->throttle;
					vehicles.at(i)->vehicle.mCommandState.brakes[0] = players[i]->playerProperties->brake;
					vehicles.at(i)->vehicle.mCommandState.nbBrakes = 1;
				}

				// Reverse Drive
				else {
					// Set inputs for reverse driving
					vehicles.at(i)->vehicle.mEngineDriveState.gearboxState.currentGear = 0;
					vehicles.at(i)->vehicle.mCommandState.throttle = players[i]->playerProperties->brake;
					vehicles.at(i)->vehicle.mCommandState.brakes[0] = players[i]->playerProperties->throttle;
					vehicles.at(i)->vehicle.mCommandState.nbBrakes = 1;
				}

				// Steering Input
				vehicles.at(i)->vehicle.mCommandState.steer = players[i]->playerProperties->steer;
			}

			// In Air
			else { 
				// Update ground flag
				vehicles.at(i)->onGround = false;
				
				// Set Rotation based on air controls
				if(gScene->raycast(vehicle_transform.p+ vehicle_transform.rotate(PxVec3(0.f, 2.f, 0.f)), vehicle_transform.rotate(PxVec3(0.f, -1.f, 0.f)), 1.f, AircontrolBuffer,PxHitFlag::eMESH_BOTH_SIDES) && AircontrolBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) //if totally upside down
					vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addTorque(vehicle_transform.rotate(PxVec3(0, 0, -players[i]->playerProperties->AirRoll)), PxForceMode().eVELOCITY_CHANGE);
				else
					vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addTorque(vehicle_transform.rotate(PxVec3(players[i]->playerProperties->AirPitch * 2.5f, players[i]->playerProperties->AirRoll * 1.0f, players[i]->playerProperties->AirRoll * -3.0f) * timestep), PxForceMode().eVELOCITY_CHANGE);
			}

			// Non-Air-Dependent Inputs
			// Boost
			if (vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().magnitude() < players[i]->playerProperties->boost_max_velocity) {
				vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle_transform.rotate(PxVec3(0.f, 0.f, players[i]->playerProperties->boost) * timestep), PxForceMode().eVELOCITY_CHANGE);
			}

			// Reset
			if (length(players[i]->transform->getLinearVelocity()) < 10.0f) {
				if (players[i]->playerProperties->reset > players[i]->playerProperties->reset_max) {
					if (vehicles.at(i)->attachedTrailers.size() > 0) {
						detachTrailer(vehicles.at(i)->attachedTrailers.at(0), vehicles.at(i), NULL);
					}
					PxVec3 oldPos = vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
					PxQuat oldRot = vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q;
					vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->setGlobalPose(PxTransform(PxVec3(oldPos.x, 10.f, oldPos.z)));
					players[i]->playerProperties->reset = 0.f;
				}
			}
			else {
				players[i]->playerProperties->reset = 0.f;
			}
		}

		// AI VEHICLE INPUT
		else {
			// Update ground flag
			if (gScene->raycast(vehicle_transform.p, vehicle_transform.rotate(PxVec3(0.f, -1.f, 0.f)), 0.7f, AircontrolBuffer)
				&& AircontrolBuffer.block.shape->getSimulationFilterData().word0 == COLLISION_FLAG_GROUND) {
				if (vehicles.at(i)->onGround == false) vehicles.at(i)->landed = true;
				vehicles.at(i)->onGround = true;
			}
			else {
				vehicles.at(i)->onGround = false;
			}

			// AI State Control
			AI_StateController(vehicles.at(i), timestep);
		}
	}

	//Forward integrate the phsyx scene by a single timestep.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	// Update Entities
	int trailerIndex = 0;
	int vehicleIndex = 0;
	for (int i = 0; i < entityList.size(); i++) {
		vec3 p;		// Position Temp
		vec3 v;		// Velocity Temp
		quat q;		// Quaternion Temp

		// TRAILERS
		if (entityList.at(i).physType == PhysType::Trailer) {
			p = toGLMVec3(trailers.at(trailerIndex)->rigidBody->getGlobalPose().p);
			q = toGLMQuat(trailers.at(trailerIndex)->rigidBody->getGlobalPose().q);
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);

			// Visual Magic for Towed Trailers
			if (trailers.at(trailerIndex)->isTowed) {

				// If trailer is being towed, modify local transforms to make it look as if its on the ground
				vec3 up = entityList.at(i).transform->getUpVector();
				PxVec3 down = PxVec3(-up.x, -up.y, -up.z);
				PxVec3 rayStart = trailers.at(trailerIndex)->rigidBody->getGlobalPose().p;
				PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
				gScene->raycast(rayStart,  down, 1.5f, TrailerBuffer, PxHitFlag::eDEFAULT, filterData);
				if (TrailerBuffer.hasAnyHits()) {
					trailers.at(trailerIndex)->groundDistance = (TrailerBuffer.block.position - rayStart).magnitude();
				}
				entityList.at(i).transform->setPosition(p + (up * clamp(1.25f - trailers.at(trailerIndex)->groundDistance, 0.0f, 10.f)) * 0.85f);

				// Rotate wheels based on trailer's forward velocity
				for (int j = 1; j < entityList.at(i).localTransforms.size(); j++) {
					float forward_velocity = dot(toGLMVec3(trailers.at(trailerIndex)->rigidBody->getLinearVelocity()), entityList.at(i).transform->getForwardVector());
					vec3 rot(forward_velocity * timer->getDeltaTime(), 0.0f, 0.0f);
					entityList.at(i).localTransforms.at(j)->setRotation(normalize(entityList.at(i).localTransforms.at(j)->getRotation() * quat(rot)));
				}
			}
			trailerIndex++;
		}

		// VEHICLES
		else if (entityList.at(i).physType == PhysType::Vehicle) {
			// Global Transform + Linear Velocity
			p = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
			q = toGLMQuat(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q);
			v = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity());
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
			entityList.at(i).transform->setLinearVelocity(v);
			gameState->entityList.at(i).nbChildEntities = vehicles.at(vehicleIndex)->attachedTrailers.size();

			// Local Wheel Transforms
			for (int j = 2; j < entityList.at(i).localTransforms.size(); j++) {
				p = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.wheelShapes[j - 2]->getLocalPose().p);
				q = toGLMQuat(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.wheelShapes[j - 2]->getLocalPose().q);
				entityList.at(i).localTransforms.at(j)->setPosition(p);
				entityList.at(i).localTransforms.at(j)->setRotation(q);
			}

			// Fan Rotation
			vec3 rot(0.0f, 0.0f, 6.0f * timer->getDeltaTime());
			entityList.at(i).localTransforms.at(1)->setRotation(normalize(entityList.at(i).localTransforms.at(1)->getRotation() * quat(rot)));

			// Ground Flag
			entityList.at(i).transform->setOnGround(vehicles.at(vehicleIndex)->onGround);
			if (vehicleIndex >= gameState->numPlayers) entityList.at(i).playerProperties->boost = vehicles.at(vehicleIndex)->aiBoost;

			// Stolen Trailer Counter
			vector<int> stolenTrailerIndices = getStolenTrailerIndices(vehicles.at(vehicleIndex));
			entityList.at(i).playerProperties->stolenTrailerIndices = stolenTrailerIndices;

			vehicleIndex++;
		}
	}

	// Update Audio
	for (int i = 0; i < vehicles.size(); i++) {
		std::string vehicleName = "vehicle_";
		vehicleName += std::to_string(i);
		float distance;

		glm::vec3 audio_position = toGLMVec3(vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
		glm::vec3 audio_velocity = gameState->findEntity(vehicleName)->transform->getLinearVelocity();
		glm::vec3 audio_forward = gameState->findEntity(vehicleName)->transform->getForwardVector();
		glm::vec3 audio_up = gameState->findEntity(vehicleName)->transform->getUpVector();

		distance = glm::length(glm::distance(gameState->listener_position, audio_position));


		if (i == 0) {
			//std::cout << "Listener updated" << std::endl;
			gameState->audio_ptr->Update3DListener(gameState->listener_position, audio_velocity, -audio_forward, audio_up);

			//gameState->audio_ptr->setVolume(vehicleName + "_tire", 1.f);
			gameState->audio_ptr->UpdateTire(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, vehicles.at(i)->onGround);
			gameState->audio_ptr->UpdateEngine(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, vehicles.at(i)->vehicle.mEngineDriveState.engineState.rotationSpeed + gameState->findEntity("vehicle_0")->playerProperties->boost);
			gameState->audio_ptr->UpdateBoost(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, gameState->findEntity("vehicle_0")->playerProperties->boost);
		}
		else {
			//gameState->audio_ptr->setVolume(vehicleName + "_tire", 1.f);
			gameState->audio_ptr->UpdateTire(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, vehicles.at(i)->onGround);
			gameState->audio_ptr->UpdateEngine(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, vehicles.at(i)->vehicle.mEngineDriveState.engineState.rotationSpeed);
			gameState->audio_ptr->UpdateBoost(vehicleName, audio_position, audio_velocity, audio_forward, audio_up, distance, vehicles.at(i)->aiBoost);
		}

		if (vehicles.at(i)->landed) {
			gameState->audio_ptr->Landing(audio_position);
			vehicles.at(i)->landed = false;
		}

		
		//std::cout << vehicleName << ": " << audio_position.x;
		//std::cout << ", " << audio_position.y;
		//std::cout << ", " << audio_position.z << std::endl;

		// For some fun horn sounds
		if (i == 0 && callback_ptrs[0]->horn1) {
			callback_ptrs[0]->horn1 = false;
			gameState->audio_ptr->hornhonk("vehicle_0", audio_position, audio_velocity, audio_forward, audio_up);
		}
		if (i == 1 && callback_ptrs[0]->horn2) {
			callback_ptrs[0]->horn2 = false;
			gameState->audio_ptr->hornhonk("vehicle_1", audio_position, audio_velocity, audio_forward, audio_up);
		}
		if (i == 2 && callback_ptrs[0]->horn3) {
			callback_ptrs[0]->horn3 = false;
			gameState->audio_ptr->hornhonk("vehicle_2", audio_position, audio_velocity, audio_forward, audio_up);
		}
		if (i == 3 && callback_ptrs[0]->horn4) {
			callback_ptrs[0]->horn4 = false;
			gameState->audio_ptr->hornhonk("vehicle_3", audio_position, audio_velocity, audio_forward, audio_up);
		}
		if (i == 4 && callback_ptrs[0]->horn5) {
			callback_ptrs[0]->horn5 = false;
			gameState->audio_ptr->hornhonk("vehicle_4", audio_position, audio_velocity, audio_forward, audio_up);
		}
		if (i == 5 && callback_ptrs[0]->horn6) {
			callback_ptrs[0]->horn6 = false;
			gameState->audio_ptr->hornhonk("vehicle_5", audio_position, audio_velocity, audio_forward, audio_up);
		} 

	}
	
	
	/*
	// Update only player
	glm::vec3 audio_position = gameState->findEntity("vehicle_0")->transform->getPosition();
	glm::vec3 audio_velocity = gameState->findEntity("vehicle_0")->transform->getLinearVelocity();
	glm::vec3 audio_forward = gameState->findEntity("vehicle_0")->transform->getForwardVector();
	glm::vec3 audio_up = gameState->findEntity("vehicle_0")->transform->getUpVector();

	float distance = glm::length(glm::distance(gameState->listener_position, audio_position));
	gameState->audio_ptr->UpdateTire("vehicle_0", audio_position, audio_velocity, audio_forward, audio_up, distance, gameState->audio_ptr->contact);
	gameState->audio_ptr->Update3DListener(gameState->listener_position, audio_velocity, audio_forward, audio_up);
	*/
	//gameState->audio_ptr->UpdateTire(listener_position, listener_velocity, listener_forward, listener_up, gameState->audio_ptr->contact);


}

// Physics ^
// ====================================================================================================================
// AI

void PhysicsSystem::AI_StateController(Vehicle* vehicle, PxReal timestep) {
	Timer* timer = &Timer::Instance();
	PxTransform vehicle_transform = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().getNormalized();
	
	AI_Stuck(vehicle);

	
	// Recharge Boost
	if (!vehicle->AI_IsBoosting) {
		if (vehicle->AI_BoostMeter < 100) {
			vehicle->AI_BoostMeter += timer->getDeltaTime() * 15.f;
		}
	}
	
	// Mid Air Correction
	if (!vehicles.at(0)->onGround) {
		float threshold = 0.2f;

		if (vehicle_transform.q.x > threshold) {
			
			// NOSE UP
			vehicle->vehicle.mPhysXState.physxActor.rigidBody->addTorque(vehicle_transform.rotate(PxVec3(-2, 0, 0) * timer->getDeltaTime()), PxForceMode().eVELOCITY_CHANGE);
		}
		else if (vehicle_transform.q.x < -threshold) {
			// NOSE DOWN
			vehicle->vehicle.mPhysXState.physxActor.rigidBody->addTorque(vehicle_transform.rotate(PxVec3(2, 0, 0) * timer->getDeltaTime()), PxForceMode().eVELOCITY_CHANGE);

		}
		

	}

	// AI Stuck somewhere
	if (vehicle->AI_IsStuck) {
		

		if (vehicle->AI_ReverseTimer < 0.25f) {
			if (vehicle->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().magnitude() < 60.f) {
				vehicle->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, -1)), PxForceMode().eVELOCITY_CHANGE);
				vehicle->vehicle.mCommandState.steer = 1.f;
			}
			vehicle->AI_ReverseTimer += timer->getDeltaTime();
		}
		else {
			vehicle->AI_IsStuck = false;
			vehicle->AI_ReverseTimer = 0.f;
		}
		
	}
	else {
		if (vehicle->AI_State == 0) {
			AI_FindTrailer(vehicle);
			AI_CollectTrailer(vehicle, timestep);
		}
		if (vehicle->AI_State == 1) {
			AI_DropOff(vehicle);
		}
		if (vehicle->AI_State == 2) {
			AI_BumpPlayer(vehicle);
		}
	}
	
	

}

// No longer needed
void PhysicsSystem::AI_InitSystem() {
	AI_State = 0;
	currTrailerIndex = 0;
}

// Deprecated ??
void PhysicsSystem::AI_UnStuck(Vehicle* vehicle) {
	Timer* timer = &Timer::Instance();
	

	vehicle->vehicle.mEngineDriveState.gearboxState.currentGear = 0;
	vehicle->vehicle.mEngineDriveState.gearboxState.targetGear = 0;
	vehicle->vehicle.mEngineDriveParams.gearBoxParams.switchTime = 0.1;
	vehicle->vehicle.mCommandState.brakes[0] = 0;
	vehicle->vehicle.mCommandState.nbBrakes = 1;
	vehicle->vehicle.mCommandState.throttle = 1.f;
	vehicle->vehicle.mCommandState.steer = 1.f;

	if (vehicle->AI_ReverseTimer < 10.f) {
		
		vehicle->AI_ReverseTimer += timer->getDeltaTime();

		
	}
	else if (vehicle->AI_ReverseTimer >= 10.f) {
		vehicle->vehicle.mEngineDriveState.gearboxState.currentGear = 2;
		vehicle->vehicle.mEngineDriveState.gearboxState.targetGear = 2;
		vehicle->AI_IsStuck = false;
		vehicle->AI_ReverseTimer = 0.f;
	}
	
}

void PhysicsSystem::AI_Stuck(Vehicle* vehicle) {
	Timer* timer = &Timer::Instance();
	if (vehicle->positions.size() < 500) {
		vehicle->positions.push_back(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
		}

	else {
		PxVec3 delta = vehicle->positions[0] - vehicle->positions[499];
		PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;

		
		if (distanceSq < 100.f) {
			
			float timeStuck = 0.f;
			vehicle->AI_IsStuck = true;
			cout << "VEHICLE STUCK" << endl;
			
		}
		
		vehicle->positions.clear();
	}
	
}



void PhysicsSystem::AI_MoveTo(Vehicle* vehicle, PxVec3 destination) {


	// CHECK IF PLAYER IS RIGHT BEHIND THEM, Last minute evade so they cant just steal
	PxVec3 delta_player = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxReal distanceSq_player = delta_player.x * delta_player.x + delta_player.z * delta_player.z;
	PxVec3 objectDirection_player = (vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();

	PxReal dotProductFront = delta_player.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));

	if (distanceSq_player < 500.f && dotProductFront <= 0) {
		AI_ApplyBoost(vehicle);
	}
	else {
		vehicle->aiBoost = 0.f;
	}
	
	vehicle->vehicle.mCommandState.steer = 0.f;
	vehicle->vehicle.mCommandState.throttle = 1.f;
	
	PxQuat pxrot = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q;
	

	glm::quat rotation;
	rotation.w = pxrot.w;
	rotation.x = pxrot.x;
	rotation.y = pxrot.y;
	rotation.z = pxrot.z;


	glm::mat4 rotationMat = glm::toMat4(rotation);

	glm::vec3 vanHeading = (rotationMat * glm::vec4(0.f, 0.f, -1.f, 1.f));

	PxVec3 pos = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxVec3 target;


	


	float offset = 0.f;

	PxVec3 vanHeadingPx;
	vanHeadingPx.x = vanHeading.x;
	vanHeadingPx.y = vanHeading.y;
	vanHeadingPx.z = vanHeading.z;

	target.x = destination.x - pos.x;
	target.y = destination.y - pos.y;
	target.z = destination.z - pos.z;


	
	

	//PxVec3 objectDirection = (currTrailer->rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
	PxVec3 objectDirection = (destination - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
	PxReal dotProduct = objectDirection.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(1, 0, 0)));



	// Trailer to the right of vehicle
	if (dotProduct > 0) {
		offset = -2.25f;
	}
	// Trailer to the left of vehicle
	if (dotProduct < 0) {
		offset = 2.25f;
	}

	target.x += offset;
	target.z += offset;

	target.normalize();

	float dot = target.dot(vanHeadingPx);


	if (vehicle->AI_DroppingOff) {

		PxVec3 delta = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - destination;

		PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;


		// If player comes up behind them while they dropping off
		PxVec3 delta_player = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
		PxReal distanceSq_player = delta_player.x * delta_player.x + delta_player.z * delta_player.z;
		PxVec3 objectDirection = (vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
		PxReal dotProduct = objectDirection.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(1, 0, 0)));

		PxReal dotProductFront = delta_player.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));


		if (distanceSq < 11000 && sqrt(dot * dot) > 0.95f) {
			//cout << "boost tech" << endl;
			AI_ApplyBoost(vehicle);
		}
		
		else if (distanceSq_player < 1000.f && dotProductFront <= 0) {
			AI_ApplyBoost(vehicle);
		}

		else {
			vehicle->aiBoost = 0.0f;
		}
	}
	if (sqrt(dot * dot) > 0.95f) {
		vehicle->vehicle.mCommandState.steer = 0.f;
	}
	else {
		PxVec3 cross = vanHeadingPx.cross(target);
		cross.normalize();
		if (cross.y < 0) {
			vehicle->vehicle.mCommandState.steer = 1.f;
		}
		else {
			vehicle->vehicle.mCommandState.steer = -1.f;
		}
	}
}

void PhysicsSystem::AI_FindTrailer(Vehicle* vehicle) {
	int tempIdx = 0;
	Timer* timer = &Timer::Instance();
	PxReal closestDistanceSq = PX_MAX_REAL;
	vehicle->AI_IsBoosting = false;
	/*if (!vehicle->AI_IsBoosting && !vehicle->AI_BoostMeter > 100.f) {
		vehicle->AI_BoostMeter += 25.f * timer->getDeltaTime();
	}*/
	
	for (int i = 0; i < trailers.size(); i++) {

		// Safety Checks: Make sure pointers aren't NULL
		if (trailers.at(i) == NULL || vehicle == NULL) {
			continue;
		}
		if (vehicle->vehicle.mPhysXState.physxActor.rigidBody == NULL) {
			continue;
		}

		// If it's attached to itself
		if (find(vehicle->attachedTrailers.begin(), vehicle->attachedTrailers.end(), trailers.at(i)) != vehicle->attachedTrailers.end()) {
			continue;
		}

		// Preference to not pick up a trailer near another person
		if (vehicle->AI_Personality == "Defensive") {
			bool trailerClose = false;
			for (Vehicle* currVehicle : vehicles) {

				if (currVehicle == vehicle)
					continue;
				// Get the distance between trailer(i) and currVehicle
				PxVec3 delta = trailers.at(i)->rigidBody->getGlobalPose().p - currVehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
				PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;

				if (distanceSq < 750.f) {
					trailerClose = true;
				}

			}
			if (trailerClose)
				continue;
		}

		/*if (vehicle->AI_Personality == "Aggressive") {

		}*/


		PxVec3 delta = trailers.at(i)->rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
		PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;


		// These two lines prevent circling
		PxReal dotProduct = delta.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));
		
		if (dotProduct <= 0) {
			continue;
		}

		if (trailers.at(i)->rigidBody->getGlobalPose().p.y > 10) {
			continue;
		}

		

		else if (distanceSq < closestDistanceSq) {

			
			
			closestDistanceSq = distanceSq;
			tempIdx = i;
			
			
		}
	}

	vehicle->AI_CurrTrailerIndex = tempIdx;
}

void PhysicsSystem::AI_CollectTrailer(Vehicle* vehicle, PxReal timestep) {


	if (vehicles.at(0)->attachedTrailers.size() > 0) {
		AI_DetermineAttackPatterns(vehicle, vehicles.at(0));
	}
	

	Trailer* currTrailer = trailers.at(vehicle->AI_CurrTrailerIndex);
	
	if (currTrailer->isTowed) {
		// Apply Mini Boost?
	}
	AI_MoveTo(vehicle, currTrailer->rigidBody->getGlobalPose().p);

	
	if (vehicle->AI_Personality == "Defensive") {
		AI_DefensiveManeuvers(vehicle, vehicles.at(0), timestep);
	}




	
	// Calculating Attack Patterns
	PxVec3 delta = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;


	// If there is a player in front and close
	PxReal dotProductPlayer = delta.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));
	
	
	
	if (vehicle->attachedTrailers.size() >= vehicle->AI_DropOffThreshold) {
		vehicle->AI_State = 1;
	}

	/*if (vehicle->AI_Personality == "Defensive") {
		if (vehicle->attachedTrailers.size()) {
			vehicle->AI_State = 1;
			cout << "Defensive bot dropping off!" << endl;
		}
	}

	

	else {
		if(vehicle->attachedTrailers.size() > 4){
			vehicle->AI_State = 1;
		}
	}*/



}

void PhysicsSystem::AI_DropOff(Vehicle* vehicle) {
	vehicle->AI_DroppingOff = true;
	//vehicle->vehicle.mCommandState.throttle = 0.f;
	AI_MoveTo(vehicle, PxVec3(0.f, 0.f, 32.f));
	if (vehicle->attachedTrailers.size() == 0) {
		vehicle->AI_State = 0;
		vehicle->AI_DroppingOff = false;

		if (vehicle->AI_Personality == "Defensive") {
			vehicle->AI_DropOffThreshold = randNum(6, 9);
		}
		else {
			vehicle->AI_DropOffThreshold = randNum(3, 6);
		}
	}

}

void PhysicsSystem::AI_ApplyBoost(Vehicle* vehicle) {
	Timer* timer = &Timer::Instance();
	vehicle->AI_IsBoosting = true;
	bool allowedToBoost = true;
	if (vehicle->AI_BoostMeter < 0.f) {
		allowedToBoost = false;
	}


	if (vehicle->AI_IsBoosting && vehicle->AI_BoostMeter > 0) {
		vehicle->AI_BoostMeter = vehicle->AI_BoostMeter - 25.f * timer->getDeltaTime();
	}

	
	if (vehicle->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().magnitude() < 60.f && allowedToBoost) {
		vehicle->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)), PxForceMode().eVELOCITY_CHANGE);
		vehicle->aiBoost = 20.0f;
	}
}

void PhysicsSystem::AI_DefensiveManeuvers(Vehicle* self, Vehicle* attacker, PxReal timestep) {

	// Calc distance to attacker
	PxVec3 delta = attacker->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - self->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;
	PxVec3 objectDirection = (attacker->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - self->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
	PxReal dotProduct = objectDirection.dot(self->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(1, 0, 0)));

	PxReal dotProductFront = delta.dot(self->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));

	
	/*if (vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().magnitude() < players[0]->playerProperties->boost_max_velocity) {
		vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle_transform.rotate(PxVec3(0.f, 0.f, players[0]->playerProperties->boost) * timestep), PxForceMode().eVELOCITY_CHANGE);
	}*/

	if (distanceSq < 1000.f) { // Dangerously Close
			// If attacker to the right of self, hard right
		

		// If the attacker is behind us
		if (dotProductFront <= 0) {
			
			AI_ApplyBoost(self);
			

			// Implement a boost here
		}
		else {
			// + dot Product --> vehicle on left of AI
			// - dot Product --> Vehicle on right of AI
			if (dotProduct > 0) {
				self->vehicle.mCommandState.steer = -1.f;
			}
			else if (dotProduct < 0) {
				self->vehicle.mCommandState.steer = 1.f;
			}
			self->aiBoost = 0.0f;
		}
	}
	else {
		self->aiBoost = 0.0f;
	}
	
}

void PhysicsSystem::AI_DetermineAttackPatterns(Vehicle* vehicle, Vehicle* target) {
	PxVec3 delta = target->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;


	// If there is a player in front and close
	PxReal dotProductPlayer = delta.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));


	

	float attackDistanceModifier = 0;
	float attackAngleModifier = 0.f;
	if (vehicle->AI_Personality == "Aggressive") {
		attackDistanceModifier = 3000.f;
		attackAngleModifier = -0.5f;
	}
	if (vehicle->AI_Personality == "Default") {
		attackDistanceModifier = 1500.f;
		attackAngleModifier = -0.3f;
	}

	if (dotProductPlayer > 0 + attackAngleModifier && vehicle->AI_Personality!="Defensive") {
		if (distanceSq < 2500.f + attackDistanceModifier ) {
			vehicle->AI_State = 2;
			//cout << "ATTACK!" << endl;

		}
		
	
	
	}
}

void PhysicsSystem::AI_BumpPlayer(Vehicle* vehicle) {

	

	PxVec3 delta = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;


	// If there is a player in front and close
	PxReal dotProductPlayer = delta.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(0, 0, 1)));
	

	float attackDistanceModifier = 0;
	float attackAngleModifier = 0.f;
	if (vehicle->AI_Personality == "Aggressive") {
		attackDistanceModifier = 2000.f;
		attackAngleModifier = 0.5f;
	}
	if (dotProductPlayer <= 0 + attackAngleModifier || distanceSq >= 2500.f + attackDistanceModifier) {
		vehicle->AI_State = 0;
	}
	else {

		PxVec3 destination = vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
		vehicle->vehicle.mCommandState.steer = 0.f;
		vehicle->vehicle.mCommandState.throttle = 0.5f;

		PxQuat pxrot = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q;


		glm::quat rotation;
		rotation.w = pxrot.w;
		rotation.x = pxrot.x;
		rotation.y = pxrot.y;
		rotation.z = pxrot.z;


		glm::mat4 rotationMat = glm::toMat4(rotation);

		glm::vec3 vanHeading = (rotationMat * glm::vec4(0.f, 0.f, -1.f, 1.f));

		PxVec3 pos = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
		PxVec3 target;





		float offset = 0.f;

		PxVec3 vanHeadingPx;
		vanHeadingPx.x = vanHeading.x;
		vanHeadingPx.y = vanHeading.y;
		vanHeadingPx.z = vanHeading.z;

		target.x = destination.x - pos.x;
		target.y = destination.y - pos.y;
		target.z = destination.z - pos.z;

		//PxVec3 objectDirection = (currTrailer->rigidBody->getGlobalPose().p - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
		PxVec3 objectDirection = (destination - vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p).getNormalized();
		PxReal dotProduct = objectDirection.dot(vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.rotate(PxVec3(1, 0, 0)));



		// Trailer to the right of vehicle
		if (dotProduct > 0) {
			offset = -2.25f;
		}
		// Trailer to the left of vehicle
		if (dotProduct < 0) {
			offset = 2.25f;
		}

		target.x += offset;
		target.z += offset;

		target.normalize();

		float dot = target.dot(vanHeadingPx);
		if (sqrt(dot * dot) > 0.95f) {
			// In front
			vehicle->vehicle.mCommandState.steer = 0.f;

			// Calculate distance
			PxReal distanceSq = delta.x * delta.x + delta.z * delta.z;
			if (distanceSq < 500.f) {
				AI_ApplyBoost(vehicle);
			}
			else {
				vehicle->aiBoost = 0.0f;
			}
		}
		else {
			vehicle->aiBoost = 0.0f;
			PxVec3 cross = vanHeadingPx.cross(target);
			cross.normalize();
			if (cross.y < 0) {
				vehicle->vehicle.mCommandState.steer = 1.f;
			}
			else {
				vehicle->vehicle.mCommandState.steer = -1.f;
			}
		}

		//AI_MoveTo(vehicle, vehicles.at(0)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
		

	}
}

