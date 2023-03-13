#include "AudioManager.h"


void AudioManager::Init() {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bank_path_actions, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(e_pod_pickup); 
	audioEngine.LoadEvent(e_dropoff);

	//audioEngine.LoadEvent(e_tire_roll);
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_0");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_1");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_2");
	audioEngine.LoadEventInstanced(e_tire_roll, "vehicle_3");





	audioEngine.LoadSound("assets/audio/ping_placeholder.flac");
	audioEngine.LoadSound("assets/audio/Latch1.wav");
	audioEngine.LoadSound("assets/audio/SpaceMusic1.wav", false, true, false);
	audioEngine.PlaySound("assets/audio/SpaceMusic1.wav", glm::vec3(0.0f), 1.0f);


	//audioEngine.PlayEvent("vehicle_0");
	audioEngine.PlayEvent("vehicle_1");
	audioEngine.PlayEvent("vehicle_2");
	audioEngine.PlayEvent("vehicle_3");

}



void AudioManager::Update() {
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
	if (contact) {
		speed = glm::length(velocity);
	}
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

	audioEngine.SetEvent3dAttributes(strEventName, pos, velocity, forward, up);
	audioEngine.SetEventParameter(strEventName, "Speed", speed);
	audioEngine.SetEventParameter(strEventName, "Distance", distance);
}








