#include <RenderingSystem.h>

// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	RenderingSystem renderer = RenderingSystem();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Process Callbacks
		std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr);
	}

	// Terminate program
	glfwTerminate();
	return 0;
}