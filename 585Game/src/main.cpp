#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>
#include <GameState.h>
#include <Boilerplate/Timer.h>
#include "NavMesh.h"
#include "AiController.h"
#include "Boilerplate/AudioEngine.h"


// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	// Systems Creation
	Timer* timer = &Timer::Instance();
	GameState* gameState = new GameState();
	RenderingSystem renderer = RenderingSystem();
	PhysicsSystem physics = PhysicsSystem();
	XboxInput xInput;
	AiController* aiController =  new AiController();
	CAudioEngine audio;

	// Flags
	bool isLoaded = false;	// false if first render update hasn't finished, true otherwise

	// Initialize Systems
	xInput.run();
	gameState->initGameState();
	physics.initPhysicsSystem(gameState, aiController);
	aiController->initAiSystem(gameState);
	audio.Init();

	//aiController.initAiSystem(gameState, gameState->findEntity("vehicle_1"));
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();

	std::string bankPathMaster = "assets/audio/Master.bank";
	std::string bankPathTest = "assets/audio/testbank.bank";
	//std::string eventName = "event:/testing_1";
	std::string eventName = "{800f3d36-fb85-49e9-909d-312439b0f460}";

	audio.LoadBank(bankPathMaster, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audio.LoadBank(bankPathTest, FMOD_STUDIO_LOAD_BANK_NORMAL);
	audio.LoadEvent(eventName);
	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {

		// Post-Load
		if (isLoaded) {
			// Update Timer
			timer->update();

			// End Game if Time is Up
			if (timer->getCountdown() <= 0 && !gameState->gameEnded) {
				gameState->endGame();
			}

			// Update Physics System
			physics.stepPhysics(callback_ptr, timer);
			aiController->StateController();

		}

		// Update Input Drivers
		xInput.update();
		callback_ptr->XboxUpdate(xInput, timer);


		if (callback_ptr->audioTest) {
			std::cout << "Audio Test 1" << std::endl;
			audio.PlayEvent(eventName);
			callback_ptr->audioTest = false;
		}
		audio.Update();

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, gameState, timer);

		// Post-Load Initialization
		if (!isLoaded) {
			timer->init();
			isLoaded = true;
		}
	


	}
	
	// Terminate program
	audio.Shutdown();
	renderer.shutdownImgui();
	xInput.stop();
	glfwTerminate();
	return 0;
}