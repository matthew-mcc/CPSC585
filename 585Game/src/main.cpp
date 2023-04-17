#pragma once
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")		// Comment out to keep debug terminal window
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
	audio.Init(gameState->numVehicles);	// Remember to change this if we ever find a better way to keep track of vehicle count
	vector<std::shared_ptr<CallbackInterface>> callback_ptrs;
	callback_ptrs.push_back(processInput(renderer.window, timer));
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
			physics.stepPhysics(callback_ptrs, timer);
			aiController->StateController();
		}

		// Update Input Drivers
		xInput.update();
		if (gameState->inMenu) {
			isLoaded = false;
			for (int i = 0; i < callback_ptrs.size(); i++) callback_ptrs[i]->XboxUpdate(xInput, 0.0f, gameState->gameEnded, i);
			gameState->menuEventHandler(callback_ptrs);
		}
		else {
			for (int i = 0; i < callback_ptrs.size(); i++) callback_ptrs[i]->XboxUpdate(xInput, length(gameState->findEntity("vehicle_" + to_string(i))->transform->getLinearVelocity()), gameState->gameEnded, i);
		}

		if (gameState->gameEnded && callback_ptrs[0]->backToMenu) {
			audio.audioEngine.StopEvent("SpaceMusic2");
			audio.audioEngine.PlayEvent("SpaceIntro");
		}

		// Update Audio Manager
		audio.Update(gameState->numVehicles, gameState->inMenu);

		// Update Rendering System
		renderer.updateRenderer(callback_ptrs, gameState, timer);

		// Menu to Game Loading
		if (!isLoaded && !gameState->inMenu) {
			gameState->resetGameState(audio_ptr);
			aiController = new AiController();
			aiController->initAiSystem(gameState);
			physics.initPhysicsSystem(gameState, aiController);
			timer->init();
			renderer.resetRenderer();
			audio.StartEvents(gameState->numVehicles);
			xInput.run(gameState->numPlayers);
			for (int i = 1; i < gameState->numPlayers; i++) callback_ptrs.push_back(processInput(renderer.window, timer));
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