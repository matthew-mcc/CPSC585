#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>
#include <GameState.h>
#include <Boilerplate/Timer.h>
#include "NavMesh.h"
#include "AiController.h"
#include "AudioManager.h"


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
	AudioManager audio;
	AudioManager* audio_ptr = &audio;

	// Flags
	bool isLoaded = false;	// false if first render update hasn't finished, true otherwise

	// Initialize Systems
	xInput.run();
	audio.Init();
	gameState->initGameState(audio_ptr);
	physics.initPhysicsSystem(gameState, aiController);
	aiController->initAiSystem(gameState);

	//aiController.initAiSystem(gameState, gameState->findEntity("vehicle_1"));
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();


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
		callback_ptr->XboxUpdate(xInput, timer, length(gameState->findEntity("vehicle_0")->transform->getLinearVelocity()));

		// Update Audio Manager
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