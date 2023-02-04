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
	RenderingSystem renderer = RenderingSystem();
	PhysicsSystem physics = PhysicsSystem();
	GameState* gameState = new GameState();
	XboxInput xInput;

	// Initialize Systems
	xInput.run();
	gameState->initGameState();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Process Callbacks
		std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
		xInput.update();
		callback_ptr->XboxUpdate(xInput);

		// Update Delta Time
		timer->update();
		
		// Update Physics System
		physics.stepPhysics(callback_ptr, gameState, timer);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, gameState, timer);
	}
  
	// Terminate program
	renderer.shutdownImgui();
	xInput.stop();
	glfwTerminate();
	return 0;
}