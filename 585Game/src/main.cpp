#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>
#include <Boilerplate/Timer.h>


// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	// System Initialization
	Timer* timer = &Timer::Instance();
	RenderingSystem renderer = RenderingSystem();
	PhysicsSystem physics = PhysicsSystem();
	XboxInput xInput;
	xInput.run();

	std::vector<Entity> entityList;
	

	entityList.emplace_back();
	entityList.back().transform = new Transform();
	entityList.back().model = new Model("assets/models/tire1/tire1.obj");


	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Process Callbacks
		std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);
		xInput.update();
		callback_ptr->XboxUpdate(xInput);

		// Update Delta Time
		timer->update();
		
		// Update Physics System
		physics.stepPhysics(entityList, timer);

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, entityList, timer);


	}
  
	// Terminate program
  renderer.shutdownImgui();
	xInput.stop();
	glfwTerminate();
	return 0;
}