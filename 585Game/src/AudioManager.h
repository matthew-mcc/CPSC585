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
	void Init();
	void Update();
	void Shutdown();

	void setTestFlag();

	void SFX(std::string eventName);


	CAudioEngine audioEngine;
	CAudioEngine* audioEnginePtr;


private:
	// Bank paths
	std::string bankPathMaster = "assets/audio/Master.bank";
	std::string bankPathTest = "assets/audio/testbank.bank";

	// Event GUID's
	std::string testEvent_1 = "{800f3d36-fb85-49e9-909d-312439b0f460}";

	bool testFlag = false;

};











