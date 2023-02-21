#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>
#include <GameState.h>
#include <Boilerplate/Timer.h>


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

	// Flags
	bool isLoaded = false;	// false if first render update hasn't finished, true otherwise

	// Initialize Systems
	xInput.run();
	gameState->initGameState();
	physics.initPhysicsSystem(gameState);
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {

		// Post-Load
		if (isLoaded) {
			// Update Timer
			timer->update();

			// Update Physics System
			physics.stepPhysics(callback_ptr, timer);
		}

		// Update Input Drivers
		xInput.update();
		callback_ptr->XboxUpdate(xInput, timer);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, gameState, timer);

		// Post-Load Initialization
		if (!isLoaded) {
			timer->init();
			isLoaded = true;
		}

	}
  
	// Terminate program
	renderer.shutdownImgui();
	xInput.stop();
	glfwTerminate();
	return 0;
}