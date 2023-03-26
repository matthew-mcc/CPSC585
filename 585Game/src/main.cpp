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
	RenderingSystem renderer = RenderingSystem(gameState);
	PhysicsSystem physics;
	XboxInput xInput;
	AiController* aiController = new AiController();
	AudioManager audio;
	AudioManager* audio_ptr = &audio;

	// Flags
	bool isLoaded = false;	// false if first render update hasn't finished, true otherwise

	// Initialize Systems
	xInput.run();
	audio.Init(6);	// Remember to change this if we ever find a better way to keep track of vehicle count
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();
	physics.initPhysX();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window) && !gameState->quit) {
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
		}

		// Update Input Drivers
		xInput.update();
		if (gameState->inMenu) {
			isLoaded = false;
			callback_ptr->XboxUpdate(xInput, timer, 0.0f, gameState->gameEnded);
			gameState->menuEventHandler(callback_ptr);
		}
		else {
			callback_ptr->XboxUpdate(xInput, timer, length(gameState->findEntity("vehicle_0")->transform->getLinearVelocity()), gameState->gameEnded);
		}

		// Update Audio Manager
		audio.Update(gameState->numVehicles, gameState->inMenu);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, gameState, timer);

		// Menu to Game Loading
		if (!isLoaded && !gameState->inMenu) {
			gameState->resetGameState(audio_ptr);
			aiController = new AiController();
			aiController->initAiSystem(gameState);
			physics.initPhysicsSystem(gameState, aiController);
			timer->init();
			renderer.resetRenderer();
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