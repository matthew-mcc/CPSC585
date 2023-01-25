#include "PhysicsSystem.h"


/*
Ideally all interaction with physX will happen in this class, we should just be passing transforms to here.
*/

PhysicsSystem::PhysicsSystem() {

	//Initialize Engine
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (!gFoundation) {
		std::cout << "PxCreateFoundation Failure!" << std::endl;
	}

	gPvd = PxCreatePvd(*gFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	// Generate Physics
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);
	if (!gPhysics) {
		std::cout << "PxCreatePhysics Failure!" << std::endl;
	}

	// Scene
	physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.f, -9.81f, 0.f);
	gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	//prep PVD
	physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}



	//Simulate Boxes
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.5f);
	physx::PxRigidStatic* groundPlane = physx::PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 50), *gMaterial);
	gScene->addActor(*groundPlane);


	float halfLen = 0.5f;
	physx::PxShape* shape = gPhysics->createShape(physx::PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
	physx::PxU32 size = 30;
	physx::PxTransform tran(physx::PxVec3(0));

	rigidDynamicList.reserve(465); // Save space for all boxes 
	//transformList.reserve(465);

	// Creating local transformations and bodies in the physics engine
	for (physx::PxU32 i = 0; i < size; i++) {
		for (physx::PxU32 j = 0; j < size - i; j++) {

			physx::PxTransform localTran(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 - 1), 0));
			physx::PxRigidDynamic* body = gPhysics->createRigidDynamic(tran.transform(localTran));

			// filling up our vectors with the new data 
			rigidDynamicList.push_back(body);
			//transformList.emplace_back(new Transform());

			body->attachShape(*shape);
			physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}


	
	shape->release(); // cleanup

	updateTransforms();

}

physx::PxVec3 PhysicsSystem::getPosition() {
	physx::PxVec3 position = rigidDynamicList[50]->getGlobalPose().p;

	return position;
}

void PhysicsSystem::updateTransforms() {
	/*for (int i = 0; i < transformList.size(); i++) {
		transformList[i].position.x = rigidDynamicList[i]->getGlobalPose().p.x;
		transformList[i].position.y = rigidDynamicList[i]->getGlobalPose().p.y;
		transformList[i].position.z = rigidDynamicList[i]->getGlobalPose().p.z;

		transformList[i].rotation.x = rigidDynamicList[i]->getGlobalPose().q.x;
		transformList[i].rotation.y = rigidDynamicList[i]->getGlobalPose().q.y;
		transformList[i].rotation.z = rigidDynamicList[i]->getGlobalPose().q.z;
		transformList[i].rotation.w = rigidDynamicList[i]->getGlobalPose().q.w;
	}*/
}