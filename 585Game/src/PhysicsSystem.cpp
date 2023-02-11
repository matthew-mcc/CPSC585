#include <ctype.h>

#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include "../snippetvehicle2common/enginedrivetrain/EngineDrivetrain.h"
#include "../snippetvehicle2common/serialization/BaseSerialization.h"
#include "../snippetvehicle2common/serialization/EngineDrivetrainSerialization.h"
#include "../snippetvehicle2common/SnippetVehicleHelpers.h"

#include "../snippetcommon/SnippetPVD.h"
#include "PhysicsSystem.h"
#include "Boilerplate/OBJ_Loader.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle2;
using namespace glm;
using namespace std;


class ContactReportCallback : public physx::PxSimulationEventCallback {
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
		PX_UNUSED(pairHeader);
		PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);

		cout << "Collision Detected!" << endl;
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

// box
PxRigidDynamic* boxBody;

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

	float halfLen = 0.5f;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
	shape->setSimulationFilterData(boxFilter);
	PxTransform tran(PxVec3(0));
	PxTransform localTran(PxVec3(10, 10, 10));
	boxBody = gPhysics->createRigidDynamic(tran.transform(localTran));
	boxBody->attachShape(*shape);
	gScene->addActor(*boxBody);

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

void initStaticMeshes(GameState* gameState) {

	// Loop through every entity, check if it's Physics Type is StaticMesh
	for (int i = 0; i < gameState->entityList.size(); i++) {
		if (gameState->entityList.at(i).type == PhysType::StaticMesh) {

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

				// Mesh description for triangle mesh
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

				// 
				PxTriangleMeshGeometry meshGeo = PxTriangleMeshGeometry(mesh, PxMeshScale(1));
				PxShape* meshShape = gPhysics->createShape(meshGeo, *gMaterial, true);

				PxFilterData meshFilter(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
				meshShape->setSimulationFilterData(meshFilter);


				PxTransform meshTrans(physx::PxVec3(0, 0, 0), PxQuat(PxIdentity));
				PxRigidStatic* meshStatic = gPhysics->createRigidStatic(meshTrans);


				meshStatic->attachShape(*meshShape);
				gScene->addActor(*meshStatic);
			}
		}
	}
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
	PxTransform pose(PxVec3(-10.f, -5.f, 0.f), PxQuat(PxIdentity));
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

	// Vehicle flags
	PxFilterData vehicleFilter(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	//PxFilterData vehicleFilter(COLLISION_FLAG_WHEEL, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	PxU32 shapes = gVehicle.mPhysXState.physxActor.rigidBody->getNbShapes();
	for (PxU32 i = 0; i < 1; i++) {
		PxShape* shape = NULL;
		gVehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);
		shape->setSimulationFilterData(vehicleFilter);

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

	}

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

void PhysicsSystem::initPhysicsSystem(GameState* gameState) {
	initPhysX();
	initStaticMeshes(gameState);
	initMaterialFrictionTable();
	initVehicles();
}

void PhysicsSystem::stepPhysics(shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer) {
	// Update Timestep
	PxReal timestep;
	if (timer->getDeltaTime() > 0.1) {	// Safety check: If deltaTime gets too large, default it to (1 / 60)
		timestep = (1 / 60.f);
	}
	else {	// Otherwise set as deltaTime as normal
		timestep = (float)timer->getDeltaTime();
	}

	// Store entity list
	auto entityList = gameState->entityList;

	//Apply the brake, throttle and steer inputs to the vehicle's command state
	gVehicle.mCommandState.brakes[0] = callback_ptr->brake;
	gVehicle.mCommandState.nbBrakes = 1;
	gVehicle.mCommandState.throttle = callback_ptr->throttle;
	gVehicle.mCommandState.steer = callback_ptr->steer;

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

	// Update Entities
	for (int i = 0; i < entityList.size(); i++) {
		vec3 p;		// Position Temp
		vec3 v;		// Velocity Temp
		quat q;		// Quaternion Temp
		
		// RIGID BODIES
		if (entityList.at(i).type == PhysType::RigidBody) {
			p = toGLMVec3(boxBody->getGlobalPose().p);
			q = toGLMQuat(boxBody->getGlobalPose().q);
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
		}

		// VEHICLES
		else if (entityList.at(i).type == PhysType::Vehicle) {
			// Global Transform + Linear Velocity
			p = toGLMVec3(gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
			q = toGLMQuat(gVehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q);
			v = toGLMVec3(gVehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity());
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
			entityList.at(i).transform->setLinearVelocity(v);

			// Local Wheel Transforms
			for (int j = 1; j < entityList.at(i).localTransforms.size(); j++) {
				p = toGLMVec3(gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p);
				q = toGLMQuat(gVehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q);
				entityList.at(i).localTransforms.at(j)->setPosition(p);
				entityList.at(i).localTransforms.at(j)->setRotation(q);
			}
		}
	}
}