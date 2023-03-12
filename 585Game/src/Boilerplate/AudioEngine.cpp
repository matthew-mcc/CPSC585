#include "AudioEngine.h"


// Error checks if all FMOD calls are successful
Instance::Instance() {
	mpStudioSystem = NULL;
	CAudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
	CAudioEngine::ErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));

	mpSystem = NULL;
	CAudioEngine::ErrorCheck(mpStudioSystem->getCoreSystem(&mpSystem));
}

// If a channel has stopped playing, destroy and clean up
Instance::~Instance() {
	CAudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
	CAudioEngine::ErrorCheck(mpStudioSystem->release());
}

// Updates the event sounds
void Instance::Update() {
	std::vector<ChannelMap::iterator> pStoppedChannels;
	for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it) {
		bool bIsPlaying = false;
		it->second->isPlaying(&bIsPlaying);
		if (!bIsPlaying) {
			pStoppedChannels.push_back(it);
		}
	}
	for (auto& it : pStoppedChannels){
		mChannels.erase(it);
	}
	CAudioEngine::ErrorCheck(mpStudioSystem->update());
}


void Instance::UpdateListenerAttributes(const FMOD_3D_ATTRIBUTES &attributes) {
	CAudioEngine::ErrorCheck(mpStudioSystem->setListenerAttributes(0, &attributes));
}


// Instance of implementation to be used
Instance* sgpImplementation = nullptr;

void CAudioEngine::Init() {
	sgpImplementation = new Instance;
}

void CAudioEngine::Update() {
	sgpImplementation->Update();
}

void CAudioEngine::UpdateListenerAttributes(const glm::vec3 &pos, const glm::vec3 &velocity, const glm::vec3 &forward, const glm::vec3 &up) {
	listener_attributes = Gen3DAttributes(pos, velocity, forward, up);
	sgpImplementation->UpdateListenerAttributes(listener_attributes);
}

// Load sounds: takes in parameters on streaming, looping, 3d and stores into sound map
void CAudioEngine::LoadSound(const std::string &strSoundName, bool b3d, bool bLooping, bool bStream) {
	auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
	if (tFoundIt != sgpImplementation->mSounds.end())
		return;

	FMOD_MODE eMode = FMOD_DEFAULT;
	eMode |= b3d ? FMOD_3D : FMOD_2D;
	eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD::Sound* pSound = nullptr;
	CAudioEngine::ErrorCheck(sgpImplementation->mpSystem->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
	if (pSound) {
		sgpImplementation->mSounds[strSoundName] = pSound;
	}
}

// Unloads sound via file name
void CAudioEngine::UnLoadSound(const std::string &strSoundName) {
	auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
	if (tFoundIt == sgpImplementation->mSounds.end())
		return;

	CAudioEngine::ErrorCheck(tFoundIt->second->release());
	sgpImplementation->mSounds.erase(tFoundIt);
}


// Plays sound
int CAudioEngine::PlaySound(const std::string &strSoundName, const glm::vec3 &vPosition, float fVolumedB) {
	// Checks if sound is in sound map, if not we load it. If not found, then error
	int nChannelID = sgpImplementation->mnNextChannelID++;
	auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);

	if (tFoundIt == sgpImplementation->mSounds.end())
	{
		LoadSound(strSoundName);
		tFoundIt = sgpImplementation->mSounds.find(strSoundName);
		if (tFoundIt == sgpImplementation->mSounds.end())
			return nChannelID;
	}

	// If found, create a new channel to house the sound and play it. Starts paused to avoid pop
	FMOD::Channel* pChannel = nullptr;
	CAudioEngine::ErrorCheck(sgpImplementation->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));
	if (pChannel) {
		FMOD_MODE currMode;
		tFoundIt->second->getMode(&currMode);
		if (currMode & FMOD_3D) {
			FMOD_VECTOR position = VectorToFmod(vPosition);
			CAudioEngine::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
		}
		CAudioEngine::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
		CAudioEngine::ErrorCheck(pChannel->setPaused(false));
		sgpImplementation->mChannels[nChannelID] = pChannel;
	}
	// ID returned in case we need it later
	return nChannelID;
}

void CAudioEngine::SetEvent3dAttributes(const string& strEventName, const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3& up) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt != sgpImplementation->mEvents.end())
		return;
	FMOD_3D_ATTRIBUTES attributes;
	attributes = Gen3DAttributes(pos, velocity, forward, up);


}


void CAudioEngine::SetChannel3dPosition(int nChannelID, const glm::vec3 &vPosition) {
	auto tFoundIt = sgpImplementation->mChannels.find(nChannelID);
	if (tFoundIt == sgpImplementation->mChannels.end())
		return;

	FMOD_VECTOR position = VectorToFmod(vPosition);
	CAudioEngine::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));

}

void CAudioEngine::SetChannelVolume(int nChannelID, float fVolumedB) {
	auto tFoundIt = sgpImplementation->mChannels.find(nChannelID);
	if (tFoundIt == sgpImplementation->mChannels.end())
		return;
	CAudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}


// Load banks - where sound information is stored for each event
void CAudioEngine::LoadBank(const std::string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
	auto tFoundIt = sgpImplementation->mBanks.find(strBankName);
	// If already loaded, then return
	if (tFoundIt != sgpImplementation->mBanks.end())
		return;

	FMOD::Studio::Bank* pBank;
	CAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &pBank));
	if (pBank) {
		std::cout << "Bank loaded successfully" << std::endl;
		sgpImplementation->mBanks[strBankName] = pBank;
	}
}


void CAudioEngine::LoadEvent(const std::string &strEventName) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt != sgpImplementation->mEvents.end())
		return;

	// This can't be right, but wtf should it be?
	FMOD::Studio::EventDescription* pEventDescription = NULL;
	CAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->getEvent(strEventName.c_str(), &pEventDescription));
	if (pEventDescription) {
		FMOD::Studio::EventInstance* pEventInstance = NULL;
		CAudioEngine::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
		if (pEventInstance) {
			sgpImplementation->mEvents[strEventName] = pEventInstance;
		}
	}
}


void CAudioEngine::PlayEvent(const std::string &strEventName) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt == sgpImplementation->mEvents.end()) {
		LoadEvent(strEventName);
		tFoundIt = sgpImplementation->mEvents.find(strEventName);
		if (tFoundIt == sgpImplementation->mEvents.end())
			return;
	}
	tFoundIt->second->start();
}


void CAudioEngine::StopEvent(const std::string &strEventName, bool bImmediate) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt == sgpImplementation->mEvents.end())
		return;

	FMOD_STUDIO_STOP_MODE eMode;
	eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
	CAudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}


bool CAudioEngine::isEventPlaying(const std::string &strEventName) const {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt == sgpImplementation->mEvents.end())
		return false;

	FMOD_STUDIO_PLAYBACK_STATE* state = NULL;
	if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
		return true;
	}
	return false;
}


void CAudioEngine::GetEventParameter(const std::string &strEventName, const std::string &strParameterName, float* parameter) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt == sgpImplementation->mEvents.end())
		return;

	// I have no clue why this doesn't exist. Need to look through documentation to get an idea of why
	// Deprecated in fmod 2.0 something -> what's new 200
	// todo: check out what this shit is supposed to do and update with new documentation
	//FMOD::Studio::ParameterInstance* pParameter = NULL;
	//CAudioEngine::ErrorCheck(tFoundIt->second->getParameterByID(strParameterName.c_str(), &pParameter));
	//CAudioEngine::ErrorCheck(pParameter->getValue(parameter));
}

// Same thing with the parameter instance issue
void CAudioEngine::SetEventParameter(const std::string &strEventName, const std::string &paramName, float fValue) {
	auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
	if (tFoundIt == sgpImplementation->mEvents.end())
		return;
	FMOD_STUDIO_PARAMETER_ID id;
	


	tFoundIt->second->setParameterByName(paramName.c_str(), fValue, false);
	
	//FMOD::Studio::EventDescription::getParameterLabelByIndex();
	//FMOD::Studio::FMOD_STUDIO_PARAMETER_ID* pParameter = NULL;
	//FMOD::Studio::ParameterInstance* pParameter = NULL;
	//CAudioEngine::ErrorCheck(tFoundIt->second->setProperty(index, fValue));
	//CAudioEngine::ErrorCheck(pParameter->setValue(parameter));
}


// All following functions used to convert linear volume to dBs
FMOD_VECTOR CAudioEngine::VectorToFmod(const glm::vec3& vPosition) {
	FMOD_VECTOR fVec;
	fVec.x = vPosition.x;
	fVec.y = vPosition.y;
	fVec.z = vPosition.z;
	return fVec;
}

FMOD_3D_ATTRIBUTES CAudioEngine::Gen3DAttributes(const glm::vec3& pos, const glm::vec3& velocity, const glm::vec3& forward, const glm::vec3 up) {
	FMOD_3D_ATTRIBUTES attributes;
	FMOD_VECTOR fpos = VectorToFmod(pos);
	FMOD_VECTOR fvelocity = VectorToFmod(velocity);
	FMOD_VECTOR fforward = VectorToFmod(forward);
	FMOD_VECTOR fup = VectorToFmod(up);

	attributes.position = fpos;
	attributes.velocity = fvelocity;
	attributes.forward = fforward;
	attributes.up = fup;

	return attributes;
}

float CAudioEngine::dbToVolume(float dB) {
	return powf(10.0f, 0.05f * dB);
}

float CAudioEngine::VolumeTodb(float volume) {
	return 20.0f * log10f(volume);
}

// ** important **
int CAudioEngine::ErrorCheck(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		std::cout << "FMOD ERROR" << result << std::endl;
		return 1;
	}
	//std::cout << "FMOD good :)" << result << std::endl;
	return 0;
}

void CAudioEngine::Shutdown() {
	delete sgpImplementation;
}











