// Following tutorial from: codyclaborn.me/tutorials/making-a-basic-fmod-audio-engine-in-c/

#pragma once

#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

// idk if I want this here
using namespace std;

// tutorial uses this to place sound in 3d space, but we use vec3 so I don't like this
struct Vector3 {
	float x;
	float y;
	float z;

};

struct Instance {
	Instance();
	~Instance();

	void Update();

	FMOD::Studio::System* mpStudioSystem;
	FMOD::System* mpSystem;

	int mnNextChannelID;

	typedef map<string, FMOD::Sound*> SoundMap;
	typedef map<int, FMOD::Channel*> ChannelMap;
	typedef map<string, FMOD::Studio::EventInstance*> EventMap;
	typedef map<string, FMOD::Studio::Bank*> BankMap;

	BankMap mBanks;
	EventMap mEvents;
	SoundMap mSounds;
	ChannelMap mChannels;
};

class CAudioEngine {
public:
	static void Init();
	static void Update();
	static void Shutdown();
	static int ErrorCheck(FMOD_RESULT result);

	void LoadBank(const string &strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
	void LoadEvent(const string &strEventName);
	void LoadSound(const string &strSoundName, bool b3d = true, bool blooping = false, bool bStream = false);
	void UnLoadSound(const string &strSoundName);
	void Set3dListenerAndOrientation(const glm::vec3 &vPos = glm::vec3(0.0f), float fVolumedB = 0.0f);
	int PlaySound(const string &strSoundName, const glm::vec3 &vPos = glm::vec3(0.0f), float fVolumedB = 0.0f);
	void PlayEvent(const string &strEventName);
	void StopChannel(int nChannelID);
	void StopEvent(const string &strEventName, bool bImmediate = false);
	void GetEventParameter(const string &strEventName, const string &strEventParameter, float* parameter);
	void SetEventParameter(const string &strEventName, const string &strParameterName, float fValue);
	void StopAllChannels();
	void SetChannel3dPosition(int nChannelID, const glm::vec3 &vPos);
	void SetChannelVolume(int nChannelID, float fVolumedB);
	bool isPlaying(int nChannelID) const;
	bool isEventPlaying(const string &strEventName) const;
	float dbToVolume(float db);
	float VolumeTodb(float volume);
	FMOD_VECTOR VectorToFmod(const glm::vec3 &vPosition);



};


#endif





