#include "AudioManager.h"


void AudioManager::Init() {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bank_path_actions, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(e_pod_pickup); 
	audioEngine.LoadEvent(e_dropoff);

	audioEngine.LoadSound("assets/audio/ping_placeholder.flac");
	audioEngine.LoadSound("assets/audio/Latch1.wav");
}



void AudioManager::Update() {
	//if (testFlag) {
	//	testFlag = false;
	//	audioEngine.PlayEvent(testEvent_1);
	//}
	audioEngine.Update();
	
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
	//float distance = glm::distance(playerPos, targetPos);
	//audioEngine.PlayEvent(e_pod_pickup);
	//audioEngine.SetEventParameter(e_pod_pickup, p_distance, distance);

	audioEngine.PlaySound("assets/audio/Latch1.wav", pos, 10.0f);


}


void AudioManager::Dropoff(glm::vec3 pos) {
	//float distance = glm::distance(playerPos, glm::vec3(1.0f, 0.5f, 33.0f));
	//audioEngine.SetEventParameter(e_dropoff, p_distance, distance);

	//float result;
	//audioEngine.SetEventParameter(e_dropoff, "Distance", distance);
	//audioEngine.SetChannel3dPosition(0, glm::vec3(1.0f, 0.5f, 33.0f));
	//audioEngine.PlayEvent(e_dropoff);

	audioEngine.PlaySound("assets/audio/ping_placeholder.flac", pos, 10.0f);
	
	//std::cout << distance << std::endl;
	//audioEngine.GetEventParameter(e_dropoff, "parameter: / Distance", &distance);

}










