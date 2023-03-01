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
ContactReportCallback* gContactReportCallback;


Pathfinder* pathfinder;

AiController* aiController;


// Vehicles
vector<Vehicle*> vehicles;
vector<PxVec3> vehicleStartPositions = vector<PxVec3>{
	PxVec3(0.0f, 8.0f, -250.0f),
	PxVec3(-250.0f, 8.0f, 0.0f),
	PxVec3(0.0f, 8.0f, 250.0f),
	PxVec3(250.0f, 8.0f, 0.0f)};
vector<PxQuat> vehicleStartRotations = vector<PxQuat>{
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f),
	PxQuat(0.0f, 0.0f, 0.0f, 1.0f)};

// Cooking shit
PxCooking* gCooking = NULL;
PxCookingParams* gParams;

// box
PxRigidDynamic* boxBody;

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

vector<PxRigidDynamic*> rigidBodies;
int rigidBodyAddIndex;


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

Vehicle* PhysicsSystem::getPullingVehicle(PxRigidDynamic* trailer) {
	for (int i = 0; i < vehicles.size(); i++) {
		for (int j = 0; j < vehicles.at(i)->attachedTrailers.size(); j++) {
			if (trailer == vehicles.at(i)->attachedTrailers.at(j)) {
				return vehicles.at(i);
			}
		}
	}
	return NULL;
}

void PhysicsSystem::processTrailerCollision() {
	// Reset contact flag, store trailer (1st in contact pair)
	gContactReportCallback->contactDetected = false;
	PxRigidDynamic* trailer = (PxRigidDynamic*)gContactReportCallback->contactPair.actors[0];

	// Find the colliding vehicle
	for (int i = 0; i < vehicles.size(); i++) {
		if (vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody == gContactReportCallback->contactPair.actors[1]) {
			// Find vehicle that is pulling the trailer (if one exists)
			Vehicle* pullingVehicle = getPullingVehicle(trailer);
			// If a pulling vehicle exists, detach the trailer
			if (pullingVehicle != NULL) {
				detachTrailer(trailer, pullingVehicle);
				// If the pulling vehicle and colliding vehicle are different, attach the trailer to the colliding vehicle
				if (pullingVehicle != vehicles.at(i)) {
					attachTrailer(trailer, vehicles.at(i));
				}
				return;
			}
			// Otherwise if no pulling vehicle exists, simply attach trailer to colliding vehicle
			else {
				attachTrailer(trailer, vehicles.at(i));
			}
		}
	}
}

void PhysicsSystem::spawnTrailer() {
	gameState->spawnTrailer();
	int max = 250;
	int min = -250;
	int range = max - min + 1;
	PxVec3 spawnPos = PxVec3(rand() % range + min, 20.f, rand() % range + min);

	PxShape* shape = gPhysics->createShape(PxBoxGeometry(.75f, .75f, .75f), *trailerMat);
	PxFilterData boxFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
	shape->setSimulationFilterData(boxFilter);

	PxRigidDynamic* trailer;
	trailer = gPhysics->createRigidDynamic(PxTransform(spawnPos));
	trailer->attachShape(*shape);
	trailer->setLinearDamping(1);
	trailer->setAngularDamping(1);
	trailer->setMass(2);
	gScene->addActor(*trailer);
	rigidBodies.push_back(trailer);
}

void PhysicsSystem::attachTrailer(PxRigidDynamic* trailer, Vehicle* vehicle) {
	PxRigidBody* truckBody = vehicle->vehicle.mPhysXState.physxActor.rigidBody;
	PxTransform truckTrans = truckBody->getGlobalPose();
	PxD6Joint* joint;
	PxTransform jointTransform;
	PxTransform trailerOffset;
	trailer->setLinearDamping(10);
	trailer->setAngularDamping(5);

	// First Joint, attach to truck rigidbody
	if (vehicle->attachedTrailers.size() == 0) {
		trailerOffset = PxTransform(PxVec3(0.0f, 0.75f, -3.f));
		trailer->setGlobalPose(truckTrans.transform(trailerOffset));
		jointTransform = PxTransform(trailer->getGlobalPose().p);
		joint = PxD6JointCreate(*gPhysics, truckBody, truckBody->getGlobalPose().getInverse().transform(jointTransform), trailer, trailer->getGlobalPose().getInverse().transform(jointTransform));
		joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
		joint->setTwistLimit(PxJointAngularLimitPair(-0.01f, 0.01f));
		joint->setPyramidSwingLimit(PxJointLimitPyramid(-0.8f, 0.8f, -0.01f, 0.01f));
	}

	// Not first joint, attach to last trailer in chain
	else {
		PxRigidDynamic* lastTrailer = vehicle->attachedTrailers.back();
		trailerOffset = PxTransform(PxVec3(0.0f, 0.0f, -3.f));
		trailer->setGlobalPose(lastTrailer->getGlobalPose().transform(trailerOffset));
		jointTransform = PxTransform(trailer->getGlobalPose().p);
		joint = PxD6JointCreate(*gPhysics, lastTrailer, lastTrailer->getGlobalPose().getInverse().transform(jointTransform), trailer, trailer->getGlobalPose().getInverse().transform(jointTransform));
		joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
		joint->setTwistLimit(PxJointAngularLimitPair(-0.01f, 0.01f));
		joint->setPyramidSwingLimit(PxJointLimitPyramid(-0.8f, 0.8f, -0.01f, 0.01f));
	}

	// Add new trailer to attachedTrailers tracking list
	vehicle->attachedTrailers.push_back(trailer);
	vehicle->attachedJoints.push_back(joint);
}

void PhysicsSystem::detachTrailer(PxRigidDynamic* trailer, Vehicle* vehicle) {
	// Init local vars
	int breakPoint = 0;
	vector<PxD6Joint*> newJoints;
	vector<PxRigidDynamic*> newTrailers;

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
		vehicle->attachedTrailers.at(i)->setAngularDamping(1);
		vehicle->attachedTrailers.at(i)->setLinearDamping(1);
	}

	// Set new joint and trailer arrays for vehicle
	vehicle->attachedJoints = newJoints;
	vehicle->attachedTrailers = newTrailers;
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

void PhysicsSystem::initVehicles(int vehicleCount) {
	// Init AI
	NavMesh* navMesh = new NavMesh();
	/*Pathfinder* path = new Pathfinder(navMesh);*/
	pathfinder = new Pathfinder(navMesh);
	
	
	//cout << path->navMesh->nodes->size() << endl;

	
	


	for (int i = 0; i < vehicleCount; i++) {
		// Create a new vehicle entity and physics struct
		gameState->spawnVehicle();
		vehicles.push_back(new Vehicle());

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
		
		if (i == 0){// only simulate for player
			for (PxU32 j = 0; j < shapes; j++) {
				PxShape* shape = NULL;
				vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, j);
				//if (shape == sha)continue;
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				if (j == 0) {
					shape->setContactOffset(0.7f);
					shape->setSimulationFilterData(vehicleFilter);
					shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				}
				else if (j == 1 || j == 6) {
					shape->setContactOffset(0.03f);
					shape->setSimulationFilterData(wheelFilter);
					shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
					gContactReportCallback->wheelshapes.push_back(shape);
				}
			}
		}
		else { //AI cars 
			for (PxU32 j = 0; j < 1; j++) {
				PxShape* shape = NULL;
				vehicles.back()->vehicle.mPhysXState.physxActor.rigidBody->getShapes(&shape, 1, j);
				shape->setSimulationFilterData(vehicleFilter);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
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
	srand(time(NULL));
	initPhysX();
	initPhysXMeshes();
	initMaterialFrictionTable();
	initVehicles(2);

	

	
	

	for (int i = 0; i < 30; i++) {
		spawnTrailer();
	}
}


void PhysicsSystem::commandAI(Vehicle* vehicle) {

	cout << aiController->selectedTrailer.first << endl;

	/*Node* startNode = pathfinder->navMesh->nodes->find(1)->second;
	Node* destNode = pathfinder->navMesh->nodes->find(2)->second;*/


	
	//pathfinder->search(startNode, destNode);

	Entity* aiVehicle = gameState->findEntity("vehicle_1");

	//cout << pathfinder->navMesh->findEntity(aiVehicle->transform->getPosition())->id << endl;
	vehicle->vehicle.mCommandState.throttle = 1.f;
	vehicle->vehicle.mCommandState.steer = 1.f;

	glm::quat rotation = aiVehicle->transform->getRotation();
	glm::mat4 rotationMat = glm::toMat4(rotation);

	glm::vec3 vanHeading = (rotationMat * glm::vec4(0.f, 0.f, -1.f, 1.f));

	PxVec3 pos = vehicle->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p;
	PxVec3 target;

	glm::vec3 dest = aiController->selectedTrailer.second;

	PxVec3 vanHeadingPx;
	vanHeadingPx.x = vanHeading.x;
	vanHeadingPx.y = vanHeading.y;
	vanHeadingPx.z = vanHeading.z;

	target.x = dest.x - pos.x;
	target.y = dest.y - pos.y;
	target.z = dest.z - pos.z;

	target.normalize();

	float dot = target.dot(vanHeadingPx);
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

void PhysicsSystem::stepPhysics(shared_ptr<CallbackInterface> callback_ptr, Timer* timer) {
	//cout << gContactReportCallback->AirOrNot << endl;
	// Update Timestep
	//cout << gameState->entityList[4].transform->getPosition().x << endl;
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
	if (callback_ptr->addTrailer) {
		attachTrailer(rigidBodies.at(rigidBodyAddIndex), vehicles.at(0));
		callback_ptr->addTrailer = false;
		rigidBodyAddIndex++;
	}

	// Store entity list
	auto entityList = gameState->entityList;
	
	Entity* player = gameState->findEntity("vehicle_0");

	//cout << player->transform->getPosition().x << ", " << player->transform->getPosition().y << ", " << player->transform->getPosition().z << endl;
	player->playerProperties->updateCallbacks(callback_ptr);
	// Super scuffed af. Right now, I just want to make playerProperties to work, so manually set our pointer?


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
		// PLAYER VEHICLE INPUT
		if (i == 0) {
			// On Ground
			if (!gContactReportCallback->AirOrNot) {
				//Apply the brake, forward throttle and steer inputs to the vehicle's command state
				if (forwardSpeed > 0.1f) {
					vehicles.at(i)->vehicle.mCommandState.brakes[0] = player->playerProperties->brake;
					vehicles.at(i)->vehicle.mCommandState.nbBrakes = 1;
				}
				// Switch brake input with reverse throttle
				else {
					vehicles.at(i)->vehicle.mCommandState.brakes[0] = 0;
					vehicles.at(i)->vehicle.mCommandState.nbBrakes = 1;
					vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle_transform.rotate(PxVec3(0.f, 0.f, -player->playerProperties->reverse * 0.2f)), PxForceMode().eVELOCITY_CHANGE);
				}
				vehicles.at(i)->vehicle.mCommandState.throttle = player->playerProperties->throttle;
				vehicles.at(i)->vehicle.mCommandState.steer = player->playerProperties->steer;
			}
			// In Air
			else { 
				// Set Rotation based on air controls
				vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addTorque(vehicle_transform.rotate(PxVec3(player->playerProperties->AirPitch * 0.0075f, player->playerProperties->AirRoll * 0.01f, 0.f)), PxForceMode().eVELOCITY_CHANGE);
			}

			// EXPERIMENTAL - Simple Boost
			vehicles.at(i)->vehicle.mPhysXState.physxActor.rigidBody->addForce(vehicle_transform.rotate(PxVec3(0.f, 0.f, player->playerProperties->boost)), PxForceMode().eVELOCITY_CHANGE);
		}

		// PLACEHOLDER - AI VEHICLE INPUT
		else {
			commandAI(vehicles.at(i));

			

			
			
			


			



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
			p = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().p);
			q = toGLMQuat(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getGlobalPose().q);
			v = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.rigidBody->getLinearVelocity());
			entityList.at(i).transform->setPosition(p);
			entityList.at(i).transform->setRotation(q);
			entityList.at(i).transform->setLinearVelocity(v);
			gameState->entityList.at(i).nbChildEntities = vehicles.at(vehicleIndex)->attachedTrailers.size();

			// Local Wheel Transforms
			for (int j = 1; j < entityList.at(i).localTransforms.size(); j++) {
				p = toGLMVec3(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().p);
				q = toGLMQuat(vehicles.at(vehicleIndex)->vehicle.mPhysXState.physxActor.wheelShapes[j - 1]->getLocalPose().q);
				entityList.at(i).localTransforms.at(j)->setPosition(p);
				entityList.at(i).localTransforms.at(j)->setRotation(q);
			}
			vehicleIndex++;
		}
	}
}