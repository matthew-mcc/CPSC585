#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>

// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	RenderingSystem renderer = RenderingSystem();
	PhysicsSystem physics;
	XboxInput x;
	x.run();

	// Process Callbacks
	std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
	renderer.SetupImgui();
	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		
		x.update();
		callback_ptr->XboxUpdate(x);
		// Update Rendering System
		renderer.updateRenderer(callback_ptr);
		
		// ONLY WORKS IN PVD RN
		physics.gScene->simulate(1.f / 60.f);
		physics.gScene->fetchResults(true);
	}
	renderer.shutdownImgui();
	x.stop();
	// Terminate program
	glfwTerminate();
	return 0;
}