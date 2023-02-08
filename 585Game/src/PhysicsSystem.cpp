

#include <ctype.h>

#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include "../snippetvehicle2common/enginedrivetrain/EngineDrivetrain.h"
#include "../snippetvehicle2common/serialization/BaseSerialization.h"
#include "../snippetvehicle2common/serialization/EngineDrivetrainSerialization.h"
#include "../snippetvehicle2common/SnippetVehicleHelpers.h"

#include "../snippetcommon/SnippetPVD.h"
#include "PhysicsSystem.h"
#include "OBJ_Loader.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle2;

class ContactReportCallback : public physx::PxSimulationEventCallback {
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
		PX_UNUSED(pairHeader);
		PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);

		std::cout << "Stop touching me :(" << std::endl;
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



//PhysX management class instances.
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxMaterial* gMaterial = NULL;
PxPvd* gPvd = NULL;

// Cooking shit
PxCooking* gCooking = NULL;
PxCookingParams* gParams;

//The path to the vehicle json files to be loaded.
const char* gVehicleDataPath = NULL;

//The vehicle with engine drivetrain
EngineDriveVehicle gVehicle;

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

//A ground plane to drive on (this is our landscape stuff).
PxRigidStatic* gGroundPlane = NULL;
PxTriangleMesh* groundMesh = NULL;






void initPhysX() {
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = gGravity;

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader;

	// Not sure if we need this
	ContactReportCallback* gContactReportCallback = new ContactReportCallback();
	sceneDesc.simulationEventCallback = gContactReportCallback;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxInitVehicleExtension(*gFoundation);

	// Cooking init
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
	if (!gCooking)
		std::cout << "PxCreateCooking Failed!" << std::endl;
	gParams = new PxCookingParams(PxTolerancesScale());
	gParams->meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	gParams->meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	gCooking->setParams(*gParams);


}

void cleanupPhysX() {
	PxCloseVehicleExtension();

	PX_RELEASE(gMaterial);
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

void initGroundPlane() {
	
	PxTriangleMeshGeometry groundGeo = PxTriangleMeshGeometry(groundMesh, PxMeshScale(1)); 
	PxShape* groundShape = gPhysics->createShape(groundGeo, *gMaterial, true);

	PxFilterData groundFilter(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);

	

	PxTransform groundTrans(physx::PxVec3(0, 0, 0), PxQuat(PxIdentity));
	PxRigidStatic* ground = gPhysics->createRigidStatic(groundTrans);
	
	
	ground->attachShape(*groundShape);
	gScene->addActor(*ground);
	
	
}

void initStaticMeshes() {

	objl::Loader loader;
	std::string objPath = "assets/models/landscape1/landscape_one.obj";
	loader.LoadFile(objPath);
	

	std::vector<PxVec3> vertexArr;
	std::vector<PxU32> indicesArr;
	for (int i = 0; i < loader.LoadedMeshes[0].Vertices.size(); i++) {
		vertexArr.push_back(PxVec3(loader.LoadedMeshes[0].Vertices[i].Position.X, loader.LoadedMeshes[0].Vertices[i].Position.Y, loader.LoadedMeshes[0].Vertices[i].Position.Z));
	}
	for (int i = 0; i < loader.LoadedMeshes[0].Indices.size(); i++) {
		indicesArr.push_back(loader.LoadedMeshes[0].Indices[i]);
	}

	// Mesh Description for Triangle Mesh
	PxTriangleMeshDesc meshDesc;
	meshDesc.setToDefault();

	meshDesc.points.count = (PxU32) vertexArr.size();
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = vertexArr.data();
	
	meshDesc.triangles.count = indicesArr.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	meshDesc.triangles.data = indicesArr.data();

	// Cooking stuff
	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = gCooking->cookTriangleMesh(meshDesc, writeBuffer);
	if (!status)
		std::cout << "Mesh Creation Failed" << std::endl;

	// Cook it and set to groundMesh
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	groundMesh = gPhysics->createTriangleMesh(readBuffer);
	

}

void cleanupGroundPlane() {
	gGroundPlane->release();
}

void initMaterialFrictionTable() {
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	gPhysXMaterialFrictions[0].friction = 1.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 1.0f;
	gNbPhysXMaterialFrictions = 1;
}

bool initVehicles() {
	gVehicleDataPath = "assets/vehicledata";

	//Load the params from json or set directly.
	readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", gVehicle.mBaseParams);
	setPhysXIntegrationParams(gVehicle.mBaseParams.axleDescription, gPhysXMaterialFrictions, gNbPhysXMaterialFrictions, gPhysXDefaultMaterialFriction, gVehicle.mPhysXParams);
	readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json", gVehicle.mEngineDriveParams);

	//Set the states to default.
	if (!gVehicle.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE)) {
		return false;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform pose(PxVec3(0.f, 25.f, 0.f), PxQuat(PxIdentity));
	gVehicle.setUpActor(*gScene, pose, gVehicleName);

	//Set the vehicle in 1st gear.
	gVehicle.mEngineDriveState.gearboxState.currentGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	gVehicle.mEngineDriveState.gearboxState.targetGear = gVehicle.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	//Set the vehicle to use the automatic gearbox.
	gVehicle.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

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
	return true;
}

void cleanupVehicles() {
	gVehicle.destroy();
}

void cleanupPhysics() {
	cleanupVehicles();
	cleanupGroundPlane();
	cleanupPhysX();
}

bool initPhysicsSystem() {
	initPhysX();
	initStaticMeshes();
	initGroundPlane();
	initMaterialFrictionTable();

	
	if (!initVehicles())
		return false;
	return true;
}

void PhysicsSystem::stepPhysics(std::shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer) {
	// Update timestep to deltaTime
	//const PxReal timestep = timer->getDeltaTime();
	const PxReal timestep = (1 / 60.f);

	// Store entity list
	auto entityList = gameState->entityList;

	//Apply the brake, throttle and steer inputs to the vehicle's command state
	gVehicle.mCommandState.brakes[0] = callback_ptr->brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = callback_ptr->throttle;
	gVehicle.mCommandState.steer = callback_ptr->steer;

	//std::cout << "Vehicle X: " << gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.x << "Vehicle Y: " << gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.y << "Vehicle Z: " << gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.z << std::endl;

	//Forward integrate the vehicle by a single timestep.
	//Apply substepping at low forward speed to improve simulation fidelity.
	const PxVec3 linVel = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity();
	const PxVec3 forwardDir = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	const PxReal forwardSpeed = linVel.dot(forwardDir);
	const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
	gVehicle.mComponentSequence.setSubsteps(gVehicle.mComponentSequenceSubstepGroupHandle, nbSubsteps);
	gVehicle.step(timestep, gVehicleSimulationContext);

	//Forward integrate the phsyx scene by a single timestep.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	for (int i = 0; i < entityList.size(); i++) {
		if (entityList.at(i).bphysicsEntity) {
			glm::vec3 position = glm::vec3();
			position.x = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.x;
			position.y = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.y;
			position.z = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p.z;
			entityList.at(i).transform->setPosition(position);

			glm::quat rotation = glm::quat();
			rotation.x = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.x;
			rotation.y = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.y;
			rotation.z = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.z;
			rotation.w = gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q.w;
			entityList.at(i).transform->setRotation(rotation);

			glm::vec3 linearVelocity = glm::vec3();
			linearVelocity.x = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().x;
			linearVelocity.y = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().y;
			linearVelocity.z = gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity().z;
			entityList.at(i).transform->setLinearVelocity(linearVelocity);

			// Iterate through all local transforms except the first (chassis)
			for (int j = 1; j < entityList.at(i).localTransforms.size(); j++) {
				position.x = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p.x;
				position.y = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p.y;
				position.z = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p.z;
				entityList.at(i).localTransforms.at(j)->setPosition(position);

				rotation.x = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q.x;
				rotation.y = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q.y;
				rotation.z = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q.z;
				rotation.w = gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q.w;
				entityList.at(i).localTransforms.at(j)->setRotation(rotation);
			}
		}
	}
}

PhysicsSystem::PhysicsSystem() {
	initPhysicsSystem();
}