#include "AudioManager.h"


void AudioManager::Init() {
	audioEngine.Init();
	// Load all banks
	audioEngine.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadBank(bankPathTest, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audioEngine.LoadEvent(testEvent_1);

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

	audioEngine.PlayEvent(testEvent_1);

}





