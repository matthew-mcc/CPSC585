#pragma once
#include <RenderingSystem.h>
#include <PhysicsSystem.h>
//#include <Entity.h>
// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	RenderingSystem renderer = RenderingSystem();
	//PhysicsSystem physics;
	XboxInput x;
	x.run();

	std::vector<Entity> entityList;
	

	entityList.reserve(80);
	for (int i = 0; i < 80; i++) {
		entityList.emplace_back();
		entityList.back().transform = new Transform();
		entityList.back().model = new Model("assets/models/tire1/tire1.obj");
	}


	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(renderer.window)) {
		// Process Callbacks
		std::shared_ptr<CallbackInterface> callback_ptr = processInput(renderer.window);

		x.update();
		callback_ptr->XboxUpdate(x);
		
		// ONLY WORKS IN PVD RN
		/*physics.gScene->simulate(1.f / 60.f);
		physics.gScene->fetchResults(true);
		physics.updateTransforms(entityList);*/

		// Update Rendering System
		renderer.updateRenderer(callback_ptr, entityList);


	}
	x.stop();
	// Terminate program
	glfwTerminate();
	return 0;
}