// Core Includes
#include <iostream>

// 3rd Party Includes
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

// Boilerplate Includes
#include <Boilerplate/Window.h>
#include <Boilerplate/Input.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Texture.h>
#include <Boilerplate/Timer.h>

#include <Boilerplate/stb_image.h>

using namespace std;
using namespace glm;


// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	// INITIALIZATION
	GLFWwindow* window = initWindow();
	Shader basicShader("src/Boilerplate/shaderVertex.txt", "src/Boilerplate/shaderFragment.txt");

	float testTriangle[] = {
			//Coordinates		 //Colors			//Texture Coords
			 0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f
	};

	//Vertex buffer initialization
	unsigned int testTriangleVAO = initVAO(testTriangle, sizeof(testTriangle));

	//Texture loading
	stbi_set_flip_vertically_on_load(true);
	unsigned int texture1 = generateTexture("src/Textures/container.jpg", true);
	unsigned int texture2 = generateTexture("src/Textures/awesomeface.png", false);
	basicShader.use(); 
	basicShader.setInt("texture1", 0);
	basicShader.setInt("texture2", 1);

	// Time initialization
	Timer &timer = Timer::Instance();		// Create pointer to singleton timer instance

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(window)) {
		// Process GLFW window user inputs
		processInput(window);

		// Update time
		timer.update();								// Update time instance
		double deltaTime = timer.getDeltaTime();	// Get delta time
		int fps = timer.getFPS(0.2);				// Get fps (WARNING: can be NULL!)
		if (fps != NULL) cout << fps << "\n";		// TODO: Render FPS on screen instead of printing to console

		// <game stuff goes here>

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		basicShader.use();
		glBindVertexArray(testTriangleVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		// Swap buffers, poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Terminate program
	glfwTerminate();
	return 0;
}