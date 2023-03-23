#include "AudioManager.h"


void AudioManager::Init(int vehicleCount) {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bank_path_actions, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(e_pod_pickup);
	audioEngine.LoadEvent(e_dropoff);

	//audioEngine.LoadEvent(e_tire_roll);
	for (int i = 0; i < vehicleCount; i++) {
		std::string vehicle = "vehicle_" + to_string(i);
		audioEngine.LoadEventInstanced(e_tire_roll, vehicle + "_tire");
		audioEngine.LoadEventInstanced(e_engine, vehicle + "_engine");

		audioEngine.PlayEvent(vehicle + "_tire");
		//audioEngine.PlayEvent(vehicle + "_engine");
	}
	/*
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_0_tire");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_1_tire");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_2_tire");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_3_tire");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_4_tire");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_5_tire");

	audioEngine.LoadEventInstanced(e_engine, "vehicle_0_engine");
	audioEngine.LoadEventInstanced(e_engine, "vehicle_1_engine");
	audioEngine.LoadEventInstanced(e_engine, "vehicle_2_engine");
	audioEngine.LoadEventInstanced(e_engine, "vehicle_3_engine");
	audioEngine.LoadEventInstanced(e_engine, "vehicle_4_engine");
	audioEngine.LoadEventInstanced(e_engine, "vehicle_5_engine");

	audioEngine.PlayEvent("vehicle_0_tire");
	audioEngine.PlayEvent("vehicle_1_tire");
	audioEngine.PlayEvent("vehicle_2_tire");
	audioEngine.PlayEvent("vehicle_3_tire");
	audioEngine.PlayEvent("vehicle_4_tire");
	audioEngine.PlayEvent("vehicle_5_tire");

	audioEngine.PlayEvent("vehicle_0_engine");
	audioEngine.PlayEvent("vehicle_1_engine");
	audioEngine.PlayEvent("vehicle_2_engine");
	audioEngine.PlayEvent("vehicle_3_engine");
	audioEngine.PlayEvent("vehicle_4_engine");
	audioEngine.PlayEvent("vehicle_5_engine");
	*/

	// Someday, we will change this
	//audioEngine.LoadEvent(e_boost);
	//audioEngine.PlayEvent(e_boost);

	audioEngine.LoadSound("assets/audio/ping_placeholder.flac");
	audioEngine.LoadSound("assets/audio/Latch1.wav");
	audioEngine.LoadSound("assets/audio/SpaceMusic2.wav", false, true, false);
	audioEngine.PlaySound("assets/audio/SpaceMusic2.wav", glm::vec3(0.0f), 1.0f);

	audioEngine.SetEventVolume("vehicle_0", 0.5f);
}



void AudioManager::Update(int numVehicles, bool inMenu) {
	for (int i = 0; i < numVehicles; i++) {
		string vehicleName = "vehicle_";
		vehicleName += to_string(i);
		if(inMenu) setVolume(vehicleName + "_tire", 0.f);
		else setVolume(vehicleName + "_tire", 1.f);
	}
	audioEngine.Update();
}

void AudioManager::Update3DListener(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up) {
	audioEngine.UpdateListenerAttributes(pos, velocity, forward, up);
}

void AudioManager::Shutdown() {
	audioEngine.Shutdown();
}

void AudioManager::setTestFlag() {
	testFlag = true;
}


void AudioManager::SFX(std::string eventName) {
	// Not implemented yet, going to need to figure out a better way to do this

}

void AudioManager::Latch(glm::vec3 pos) {
	audioEngine.PlaySound("assets/audio/Latch1.wav", pos, 10.0f);
}

void AudioManager::LatchEvent(glm::vec3 pos) {
	audioEngine.PlayEvent("assets/audio/Latch1.wav");
}

void AudioManager::Dropoff() {
	glm::vec3 dropoffPos = glm::vec3(1.0f, 0.5f, 33.0f);
	audioEngine.PlaySound("assets/audio/ping_placeholder.flac", dropoffPos, 10.0f);
}


void AudioManager::UpdateTire(const std::string &strEventName, const glm::vec3 &pos, const glm::vec3 &velocity, const glm::vec3 &forward, const glm::vec3 &up, float distance, bool contact) {
	float speed = 0.0f;
	float rpm;
	string tireSound = strEventName + "_tire";
	string engineSound = strEventName + "_engine";


	if (contact) {
		speed = glm::length(velocity);
	}
	rpm = speed * 100.0f;
	/*
	else {
		// In hindsight, all of this is the same as if I just -10 off transform velocity
		FMOD_3D_ATTRIBUTES attributes;
		FMOD_3D_ATTRIBUTES* attributes_ptr = &attributes;
		glm::vec3 velocity;
		audioEngine.GetEvent3dAttributes(strEventName, attributes_ptr);
		velocity.x = attributes.velocity.x;
		velocity.y = attributes.velocity.y;
		velocity.z = attributes.velocity.z;
		speed = glm::length(velocity);
		speed -= 10.0f;
		if (speed < 0.0f) {
			speed = 0.0f;
		}
	}
	*/
	

	audioEngine.SetEvent3dAttributes(tireSound, pos, velocity, forward, up);
	audioEngine.SetEventParameter(tireSound, "Speed", speed);
	audioEngine.SetEventParameter(tireSound, "Distance", distance);

	audioEngine.SetEvent3dAttributes(engineSound, pos, velocity, forward, up);
	audioEngine.SetEventParameter(engineSound, "RPM", rpm);

}

void AudioManager::UpdateBoostPlaceholder(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, float distance, float boost) {
	audioEngine.SetEvent3dAttributes(e_boost, pos, velocity, forward, up);
	audioEngine.SetEventParameter(e_boost, "Boost", boost);
}





void AudioManager::setVolume(const std::string& strEventName, float db) {
	audioEngine.SetEventVolume(strEventName, db);
}







