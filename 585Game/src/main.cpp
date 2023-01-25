#include <RenderingSystem.h>
#include <PhysicsSystem.h>
// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	RenderingSystem renderer = RenderingSystem();
	PhysicsSystem physics;


	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Process Callbacks
		std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr);
		
		// ONLY WORKS IN PVD RN
		physics.gScene->simulate(1.f / 60.f);
		physics.gScene->fetchResults(true);
	}

	// Terminate program
	glfwTerminate();
	return 0;
}