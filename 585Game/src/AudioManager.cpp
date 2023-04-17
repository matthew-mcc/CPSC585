#include "AudioManager.h"


void AudioManager::Init(int vehicleCount) {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bank_path_actions, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(e_pod_pickup);
	audioEngine.LoadEvent(e_dropoff);
	
	audioEngine.LoadEvent(a_hornhonk);


	//audioEngine.LoadEvent(e_tire_roll);
	for (int i = 0; i < vehicleCount; i++) {
		std::string vehicle = "vehicle_" + to_string(i);
		audioEngine.LoadEventInstanced(e_tire_roll, vehicle + "_tire");
		audioEngine.LoadEventInstanced(e_engine, vehicle + "_engine");
		audioEngine.LoadEventInstanced(e_boost, vehicle + "_boost");
		audioEngine.LoadEventInstanced(a_hornhonk, vehicle + "_honk");

	}

	// Loading misc sounds (not events)
	audioEngine.LoadSound("assets/audio/ping_placeholder.flac");
	audioEngine.LoadSound("assets/audio/Latch1.wav");
	audioEngine.LoadSound("assets/audio/Landing1.wav");
	audioEngine.LoadSound("assets/audio/Click1.wav");
	audioEngine.LoadSound("assets/audio/Click2.wav");
	audioEngine.LoadSound("assets/audio/Click3.wav");


	// Load and play music!
	//audioEngine.LoadSound("assets/audio/SpaceMusic2.wav", false, true, false);
	//audioEngine.PlaySound("assets/audio/SpaceMusic2.wav", glm::vec3(0.0f), 1.0f);
	audioEngine.LoadEventInstanced(spacemusic2GUID, "SpaceMusic2");
	audioEngine.LoadEventInstanced(spaceintroGUID, "SpaceIntro");
	//audioEngine.PlayEvent("SpaceMusic2");
	audioEngine.PlayEvent("SpaceIntro");

	// Updating some parameters
	// Debug and because it's annoying - ImGui will set it on change
	audioEngine.SetEventVolume("vehicle_0_engine", 0.5f);
	audioEngine.SetEventVolume("vehicle_0_tire", 0.5f);
	audioEngine.SetEvent3dAttributes(e_dropoff, glm::vec3(1.0f, 0.5f, 33.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
}

void AudioManager::StartEvents(int vehicleCount) {
	audioEngine.StopEvent("SpaceIntro");
	audioEngine.PlayEvent("SpaceMusic2");
	for (int i = 0; i < vehicleCount; i++) {
		std::string vehicle = "vehicle_" + to_string(i);
		audioEngine.PlayEvent(vehicle + "_tire");
		audioEngine.PlayEvent(vehicle + "_engine");
		audioEngine.PlayEvent(vehicle + "_boost");
	}
}

void AudioManager::Update(int numVehicles, bool inMenu) {
	for (int i = 0; i < numVehicles; i++) {
		string vehicleName = "vehicle_";
		vehicleName += to_string(i);
		if (inMenu) {
			setVolume(vehicleName + "_engine", 0.f);
			setVolume(vehicleName + "_tire", 0.f);
			setVolume(vehicleName + "_boost", 0.f);
			setVolume(vehicleName + "_honk", 0.f);
		}
		else{
			if (i == 0) {
				setVolume(vehicleName + "_engine", playerEngineVolume);
				setVolume(vehicleName + "_tire", playerTireVolume);
				setVolume(vehicleName + "_boost", playerEngineVolume);
				setVolume(vehicleName + "_honk", honkVolume);
			}
			else {
				setVolume(vehicleName + "_engine", 1.f);
				setVolume(vehicleName + "_tire", 1.f);
				setVolume(vehicleName + "_boost", 1.f);
				setVolume(vehicleName + "_honk", honkVolume);
			}
		}
	}
	audioEngine.Update();
}

void AudioManager::Update3DListener(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up) {
	audioEngine.UpdateListenerAttributes(pos, velocity, forward, up);
	audioEngine.SetEvent3dAttributes(SpaceMusic2, pos, velocity, forward, up);
}

void AudioManager::Shutdown() {
	audioEngine.Shutdown();
}

void AudioManager::setTestFlag() {
	testFlag = true;
}

void AudioManager::SFX(std::string eventName) {
	// Huh?
}

void AudioManager::Latch(glm::vec3 pos) {
	audioEngine.PlaySound("assets/audio/Latch1.wav", pos, 10.0f);
}

// Does nothing atm
void AudioManager::LatchEvent(glm::vec3 pos) {
	audioEngine.PlayEvent("assets/audio/Latch1.wav");
}

void AudioManager::Dropoff() {
	glm::vec3 dropoffPos = glm::vec3(1.0f, 0.5f, 33.0f);
	//audioEngine.PlaySound("assets/audio/ping_placeholder.flac", dropoffPos, 10.0f);
	audioEngine.PlayEvent(e_dropoff);
}

void AudioManager::Landing(glm::vec3 pos) {
	audioEngine.PlaySound("assets/audio/Landing1.wav", pos, 15.0f);
}

void AudioManager::CountdownBeep(int type, glm::vec3 pos) {
	if (type == 0) audioEngine.PlaySound("assets/audio/Beep1.wav", pos, 15.0f);
	if (type == 1) audioEngine.PlaySound("assets/audio/Beep2.wav", pos, 15.0f);
	if (type == 2) audioEngine.PlaySound("assets/audio/Beep3.wav", pos, 15.0f);
	if (type == 3) audioEngine.PlaySound("assets/audio/Beep4.wav", pos, 15.0f);
	if (type == 4) audioEngine.PlaySound("assets/audio/Beep5.wav", pos, 15.0f);
}

void AudioManager::MenuClick(int type, glm::vec3 pos) {
	if (type == 0) audioEngine.PlaySound("assets/audio/Click1.wav", pos);
	if (type == 1) audioEngine.PlaySound("assets/audio/Click2.wav", pos);
	if (type == 2) audioEngine.PlaySound("assets/audio/Click3.wav", pos);
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

	audioEngine.SetEvent3dAttributes(tireSound, pos, velocity, forward, up);
	audioEngine.SetEventParameter(tireSound, "Speed", speed);
	audioEngine.SetEventParameter(tireSound, "Distance", distance);
}

void AudioManager::UpdateBoost(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, float distance, float boost) {
	
	std::string soundName = strEventName + "_boost";
	audioEngine.SetEvent3dAttributes(soundName, pos, velocity, forward, up);
	audioEngine.SetEventParameter(soundName, "Boost", boost);
}

void AudioManager::UpdateEngine(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, float distance, float rpm) {
	std::string soundName = strEventName + "_engine";
	audioEngine.SetEvent3dAttributes(soundName, pos, velocity, forward, up);
	audioEngine.SetEventParameter(soundName, "RPM", rpm);
}

void AudioManager::setVolume(const std::string& strEventName, float db) {
	audioEngine.SetEventVolume(strEventName, db);
}

void AudioManager::hornhonk(const std::string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up) {
	std::string soundName = strEventName + "_honk";
	audioEngine.SetEvent3dAttributes(soundName, pos, velocity, forward, up);
	audioEngine.PlayEvent(soundName);
}





