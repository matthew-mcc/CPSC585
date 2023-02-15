#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "vehicle2/PxVehicleAPI.h"
#include "../snippetcommon/SnippetPVD.h"
#include "PhysicsSystem.h"
#include "Boilerplate/OBJ_Loader.h"
#include <stdlib.h>
#include <time.h> 
#include <map>


//PhysX management class instances.
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxMaterial* gMaterial = NULL;
PxMaterial* trailerMat = NULL;
PxPvd* gPvd = NULL;

// Cooking shit
PxCooking* gCooking = NULL;
PxCookingParams* gParams;

// box
PxRigidDynamic* boxBody;

//The path to the vehicle json files to be loaded.
const char* gVehicleDataPath = NULL;

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

ContactReportCallback* gContactReportCallback = new ContactReportCallback();
vector<PxRigidDynamic*> rigidBodies;
vector<PxRigidBody*> vehicleBodies;
vector<vector<PxRigidDynamic*>> attachedTrailers = { {}, {}, {}, {} };
EngineDriveVehicle vehicle1;


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

void PhysicsSystem::spawnTrailer() {
	gameState->spawnTrailer();
	int max = 300;
	int min = -300;
	int range = max - min + 1;
	PxVec3 spawnPos = PxVec3(rand() % range + min, 10.f, rand() % range + min);

	PxShape* shape = gPhysics->createShape(PxBoxGeometry(.75f, .75f, .75f), *trailerMat);
	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
	shape->setSimulationFilterData(boxFilter);

	PxRigidDynamic* trailer;
	trailer = gPhysics->createRigidDynamic(PxTransform(spawnPos));
	trailer->attachShape(*shape);
	trailer->setLinearDamping(10);
	trailer->setAngularDamping(5);
	gScene->addActor(*trailer);
	rigidBodies.push_back(trailer);
}

void PhysicsSystem::attachTrailer(PxRigidDynamic* trailer, int truckIndex) {
	PxRigidBody* truck = vehicleBodies.at(truckIndex);
	PxTransform truckTrans = truck->getGlobalPose();
	PxD6Joint* joint;
	PxTransform jointTransform;
	PxTransform trailerOffset;

	// First Joint, attach to truck rigidbody
	if (attachedTrailers.at(truckIndex).size() == 0) {
		trailerOffset = PxTransform(PxVec3(0.0f, 0.75f, -3.f));
		trailer->setGlobalPose(truckTrans.transform(trailerOffset));
		jointTransform = PxTransform(trailer->getGlobalPose().p);
		joint = PxD6JointCreate(*gPhysics, truck, truck->getGlobalPose().getInverse().transform(jointTransform), trailer, trailer->getGlobalPose().getInverse().transform(jointTransform));
		joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
		joint->setTwistLimit(PxJointAngularLimitPair(-0.01f, 0.01f));
		joint->setPyramidSwingLimit(PxJointLimitPyramid(-0.25f, 0.25f, -0.01f, 0.01f));
	}

	// Not first joint, attach to last trailer in chain
	else {
		PxRigidDynamic* lastTrailer = attachedTrailers.at(truckIndex).back();
		trailerOffset = PxTransform(PxVec3(0.0f, 0.0f, -3.f));
		trailer->setGlobalPose(lastTrailer->getGlobalPose().transform(trailerOffset));
		jointTransform = PxTransform(trailer->getGlobalPose().p);
		joint = PxD6JointCreate(*gPhysics, lastTrailer, lastTrailer->getGlobalPose().getInverse().transform(jointTransform), trailer, trailer->getGlobalPose().getInverse().transform(jointTransform));
		joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
		joint->setTwistLimit(PxJointAngularLimitPair(-0.01f, 0.01f));
		joint->setPyramidSwingLimit(PxJointLimitPyramid(-0.25f, 0.25f, -0.01f, 0.01f));
	}

	// Add new trailer to attachedTrailers tracking list
	attachedTrailers.at(truckIndex).push_back(trailer);
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

	//ContactReportCallback* gContactReportCallback = new ContactReportCallback();
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

void PhysicsSystem::initMaterialFrictionTable() {
	//Each physx material can be mapped to a tire friction value on a per tire basis.
	//If a material is encountered that is not mapped to a friction value, the friction value used is the specified default value.
	//In this snippet there is only a single material so there can only be a single mapping between material and friction.
	//In this snippet the same mapping is used by all tires.
	gPhysXMaterialFrictions[0].friction = 2.0f;
	gPhysXMaterialFrictions[0].material = gMaterial;
	gPhysXDefaultMaterialFriction = 2.0f;
	gNbPhysXMaterialFrictions = 1;
}

bool PhysicsSystem::initVehicles() {
	//Load the params from json or set directly.
	gVehicleDataPath = "assets/vehicledata";
	readBaseParamsFromJsonFile(gVehicleDataPath, "Base.json", vehicle1.mBaseParams);
	setPhysXIntegrationParams(vehicle1.mBaseParams.axleDescription, gPhysXMaterialFrictions, gNbPhysXMaterialFrictions, gPhysXDefaultMaterialFriction, vehicle1.mPhysXParams);
	readEngineDrivetrainParamsFromJsonFile(gVehicleDataPath, "EngineDrive.json", vehicle1.mEngineDriveParams);

	//Set the states to default.
	if (!vehicle1.initialize(*gPhysics, PxCookingParams(PxTolerancesScale()), *gMaterial, EngineDriveVehicle::eDIFFTYPE_FOURWHEELDRIVE)) {
		return false;
	}

	//Apply a start pose to the physx actor and add it to the physx scene.
	PxTransform pose(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity));
	vehicle1.setUpActor(*gScene, pose, gVehicleName);

	//Set the vehicle in 1st gear.
	vehicle1.mEngineDriveState.gearboxState.currentGear = vehicle1.mEngineDriveParams.gearBoxParams.neutralGear + 1;
	vehicle1.mEngineDriveState.gearboxState.targetGear = vehicle1.mEngineDriveParams.gearBoxParams.neutralGear + 1;

	//Set the vehicle to use the automatic gearbox.
	vehicle1.mTransmissionCommandState.targetGear = PxVehicleEngineDriveTransmissionCommandState::eAUTOMATIC_GEAR;

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
	PxU32 shapes = vehicle1.mPhysXState.physxActor.rigidBody->getNbShapes();
	for (PxU32 i = 0; i < 1; i++) {
		PxShape* shape = NULL;
		vehicle1.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, i);
		shape->setSimulationFilterData(vehicleFilter);

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}
	vehicleBodies.push_back(vehicle1.mPhysXState.physxActor.rigidBody);

	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	return true;
}

void PhysicsSystem::initPhysicsSystem(GameState* gameState) {
	this->gameState = gameState;
	srand(time(NULL));
	initPhysX();
	initPhysXMeshes();
	initMaterialFrictionTable();
	initVehicles();

	for (int i = 0; i < 50; i++) {
		spawnTrailer();
	}

	//attachTrailer(rigidBodies.at(0), 0);
}

void PhysicsSystem::stepPhysics(shared_ptr<CallbackInterface> callback_ptr, Timer* timer) {
	// Update Timestep
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
		PxRigidDynamic* trailer = (PxRigidDynamic*)gContactReportCallback->contactPair.actors[0];
		attachTrailer(trailer, 0);
		gContactReportCallback->contactDetected = false;
	}

	// Store entity list
	auto entityList = gameState->entityList;

	//Apply the brake, throttle and steer inputs to the vehicle's command state
	vehicle1.mCommandState.brakes[0] = callback_ptr->brake;
	vehicle1.mCommandState.nbBrakes = 1;
	vehicle1.mCommandState.throttle = callback_ptr->throttle;
	vehicle1.mCommandState.steer = callback_ptr->steer;

	//Forward integrate the vehicle by a single timestep.
	//Apply substepping at low forward speed to improve simulation fidelity.
	const PxVec3 linVel = vehicle1.mPhysXState.physxActor.rigidBody->getLinearVelocity();
	const PxVec3 forwardDir = vehicle1.mPhysXState.physxActor.rigidBody->getGlobalPose().q.getBasisVector2();
	const PxReal forwardSpeed = linVel.dot(forwardDir);
	const PxU8 nbSubsteps = (forwardSpeed < 5.0f ? 3 : 1);
	vehicle1.mComponentSequence.setSubsteps(vehicle1.mComponentSequenceSubstepGroupHandle, nbSubsteps);
	vehicle1.step(timestep, gVehicleSimulationContext);

	//Forward integrate the phsyx scene by a single timestep.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	// Update Entities
	int trailerIndex = 0;
	for (int i = 0; i < entityList.size(); i++) {
		vec3 p;		// Position Temp
		vec3 v;		// Velocity Temp
		quat q;		// Quaternion Temp
		
		// RIGID BODIES
		if (entityList.at(i).type == PhysType::RigidBody) {
			p = toGLMVec3(rigidBodies.at(trailerIndex)->getGlobalPose().p);
			q = toGLMQuat(rigidBodies.at(trailerIndex)->getGlobalPose().q);
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
			trailerIndex++;
		}

		// VEHICLES
		else if (entityList.at(i).type == PhysType::Vehicle) {
			// Global Transform + Linear Velocity
			p = toGLMVec3(vehicle1.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
			q = toGLMQuat(vehicle1.mPhysXState.physxActor.rigidBody->getGlobalPose().q);
			v = toGLMVec3(vehicle1.mPhysXState.physxActor.rigidBody->getLinearVelocity());
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
			entityList.at(i).transform->setLinearVelocity(v);

			// Local Wheel Transforms
			for (int j = 1; j < entityList.at(i).localTransforms.size(); j++) {
				p = toGLMVec3(vehicle1.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p);
				q = toGLMQuat(vehicle1.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q);
				entityList.at(i).localTransforms.at(j)->setPosition(p);
				entityList.at(i).localTransforms.at(j)->setRotation(q);
			}
		}
	}
}