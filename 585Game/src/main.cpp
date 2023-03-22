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
	audio.Init(6);	// Remember to change this if we ever find a better way to keep track of vehicle count
	gameState->initGameState(audio_ptr);
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Loaded and not in menu (regular gameplay)
		if (isLoaded && !gameState->inMenu) {
			// Update Timer
			timer->update();

			// End Game if Time is Up
			if (timer->getCountdown() <= 0 && !gameState->gameEnded) {
				gameState->endGame();
			}

			// Update Physics System
			physics.stepPhysics(callback_ptr, timer);
			aiController->StateController();

			// Update Audio Manager
			audio.Update();
		}

		// Update Input Drivers
		xInput.update();
		if (gameState->findEntity("vehicle_0") != nullptr) callback_ptr->XboxUpdate(xInput, timer, length(gameState->findEntity("vehicle_0")->transform->getLinearVelocity()));
		else callback_ptr->XboxUpdate(xInput, timer, 0.0f);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, gameState, timer);

		// Menu to Game Loading
		if (!isLoaded && !gameState->inMenu) {
			physics.initPhysicsSystem(gameState, aiController);
			aiController->initAiSystem(gameState);
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