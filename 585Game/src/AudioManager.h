#pragma once

#include "Boilerplate/AudioEngine.h"



// What kind of behaviour do we want?
// First, initialize audio engine
// We want it to keep all of our sound GUID
// We also want some parameters that can be passed around. Other systems should be able to manipulate those parameters
// Do we need 2 classes for this?
// One that manages which sounds to play and one that other classes can pass parameters around?
// Maybe it's better not to keep all these GUID's here, but rather into a JSON file
// Then we go through and load all these events.

// The existence of this is lower and lower by each passing moment
// However, we still need something to play ambient noise
// As well as translate a string keys and GUID

// Scrap all of the above. Peter has really helped in this
// We let anything start audio in physics system by having a pointer to audio engine in the gamestate

class AudioManager {
public:
	void Init(int vehicleCount);
	void StartEvents(int vehicleCount);
	void Update(int numVehicles, bool inMenu);
	void Update3DListener(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up);
	void Shutdown();

	void setTestFlag();

	void SFX(std::string eventName);
	void Latch(glm::vec3 pos);
	void LatchEvent(glm::vec3 pos);
	void Dropoff();
	void Landing(glm::vec3 pos);
	void CountdownBeep(int type, glm::vec3 pos);
	void MenuClick(int type, glm::vec3 pos);

	void UpdateTire(const std::string& strEventName, const glm::vec3 &pos, const glm::vec3 &velocity, const glm::vec3 &forward, const glm::vec3 &up, float distance, bool contact);
	void UpdateBoost(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, float distance, float boost);
	void UpdateEngine(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, float distance, float rpm);
	void hornhonk(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up);

	void setVolume(const std::string& strEventName, float db);

	CAudioEngine audioEngine;
	CAudioEngine* audioEnginePtr;

	int lastBeep = 0;

private:
	// Bank paths
	std::string bankPathMaster = "assets/audio/Master.bank";
	std::string bank_path_actions = "assets/audio/Actions.bank";

	// Event GUID's
	std::string spacemusic2GUID = "{912a0889-5a99-4ce0-9a7c-2076275b812a}";
	std::string SpaceMusic2 = "SpaceMusic2";
	std::string e_pod_pickup = "{18b007c8-1f2c-4417-90bb-989e3419b7f9}";
	std::string e_dropoff = "{8152d370-812b-42ff-8376-d6cf870fa7b0}";
	//std::string e_dropoff_path = "event: / sfx / dropoff";

	// Truck
	std::string e_tire_roll = "{7320c7ca-8c24-42ad-9aee-e1ffbe708167}";
	std::string e_engine = "{ad086051-df49-493c-b0c8-c8b603ef2eee}";
	//std::string e_engine = "{8b8b6ea8-3476-40c4-8922-1413fc23e50b}";	// Fancy and shitty engine
	std::string e_boost = "{800dca3e-ea68-4f91-90f7-1c88dbb487b0}";
	std::string a_hornhonk = "{1b293558-9d84-4647-a62a-0b35aee3b9a3}";


	std::string p_distance = "{a535cf27-0d8c-4dc4-8a7d-386ac746bd99}";

	bool testFlag = false;
};











