#include "AudioManager.h"


void AudioManager::Init() {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bank_path_actions, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(e_pod_pickup); 
	audioEngine.LoadEvent(e_dropoff);
	audioEngine.LoadEvent(e_tire_roll);

	audioEngine.LoadSound("assets/audio/ping_placeholder.flac");
	audioEngine.LoadSound("assets/audio/Latch1.wav");
	audioEngine.LoadSound("assets/audio/SpaceMusic1.wav", false, true, false);
	audioEngine.PlaySound("assets/audio/SpaceMusic1.wav", glm::vec3(0.0f), 1.0f);
	audioEngine.PlayEvent(e_tire_roll);

	
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


void AudioManager::UpdateTire(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up, bool contact) {
	float speed;
	if (contact) {
		speed = glm::length(velocity);
	}
	else {
		speed = 0.0f;
	}

	audioEngine.SetEvent3dAttributes(e_tire_roll, pos, velocity, forward, up);
	audioEngine.SetEventParameter(e_tire_roll, "Speed", speed);
}








