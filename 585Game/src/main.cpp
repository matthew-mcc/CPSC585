// Core Includes
#include <iostream>
#include <map>

// 3rd Party Includes
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


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

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	/*float testTriangle[] = {
			//Coordinates		 //Colors			//Texture Coords
			 0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f
	};*/
	float testTriangle[] = {
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f
	};

	//Vertex buffer initialization
	unsigned int testTriangleVAO = initVAO(testTriangle, sizeof(testTriangle));

	//Texture loading
	stbi_set_flip_vertically_on_load(true);
	unsigned int texture1 = generateTexture("src/Textures/container.jpg", true);
	unsigned int texture2 = generateTexture("src/Textures/nice.jpg", true);
	basicShader.use(); 
	basicShader.setInt("texture1", 0);
	basicShader.setInt("texture2", 1);

	//Coordinate transformations
	//Transforms local coords to world coords
	glEnable(GL_DEPTH_TEST);
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//Transforms world coords to camera coords
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 projection; //Transforms camera coords to clip coords
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	// Time initialization
	Timer &timer = Timer::Instance();		// Create pointer to singleton timer instance
	std::shared_ptr<CallbackInterface> Callptr = processInput(window);
	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(window)) {
		// Process GLFW window user inputs

		// Update time
		timer.update();								// Update time instance
		double deltaTime = timer.getDeltaTime();	// Get delta time
		int fps = timer.getFPS(0.2);				// Get fps (WARNING: can be NULL!)
		if (fps != NULL) cout << fps << "\n";		// TODO: Render FPS on screen instead of printing to console

		// <game stuff goes here>
		view = glm::lookAt(Callptr->camera_pos, Callptr->camera_pos + Callptr->camera_front,Callptr->camera_up);
		model = glm::rotate(model, glm::radians(50.f) * (float)deltaTime, glm::vec3(0.5f, 1.0f, 0.0f));
		basicShader.setMat4("model", model);
		basicShader.setMat4("view", view);
		basicShader.setMat4("projection", projection);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		basicShader.use();
		glBindVertexArray(testTriangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// Swap buffers, poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Terminate program
	glfwTerminate();
	return 0;
}