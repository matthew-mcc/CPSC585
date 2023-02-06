
#include "PxPhysicsAPI.h"

// FLAGS

enum Flags {
	COLLISION_FLAG_GROUND = 1 << 0,
	COLLISION_FLAG_WHEEL = 1 << 1,
	COLLISION_FLAG_CHASSIS = 1 << 2,
	COLLISION_FLAG_OBSTACLE = 1 << 3,

	COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE

};


namespace snippetvehicle2
{

physx::PxFilterFlags VehicleFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);


bool parseVehicleDataPath(int argc, const char *const* argv, const char* snippetName,
	const char*& vehicleDataPath);

}//namespace snippetvehicle2
