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
#include <Boilerplate/Renderer.h>

using namespace std;
using namespace glm;


// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	// INITIALIZATION
	GLFWwindow* window = initWindow();

	float testTriangle[] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
	};
	//Vertex buffer initialization
	unsigned int testTriangleVAO = initVAO(testTriangle, sizeof(testTriangle));
	unsigned int shaderProgram = initShaders();

	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(window)) {
		// Process GLFW window user inputs
		processInput(window);
		
		// <game stuff goes here>
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(testTriangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		// Swap buffers, poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Terminate program
	glfwTerminate();
	return 0;
}