// Core Includes
#include <iostream>
#include <map>

// 3rd Party Includes
// PHYSX
#include <PxPhysicsAPI.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

//ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Boilerplate Includes
#include <Boilerplate/Window.h>
#include <Boilerplate/Input.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Texture.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Timer.h>
#include <Boilerplate/stb_image.h>
#include <Boilerplate/Cube.h>

using namespace std;
using namespace glm;


// Main
// Handles program initialization
// Runs the primary game loop
int main() {
	// INITIALIZATION
	GLFWwindow* window = initWindow();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize font
	std::map<char, Character> Characters = initFont("assets/fonts/arial.ttf");

	// Initialize VAOs and VBOs
	unsigned int testCubeVAO, testCubeVBO;
	initGeomVAO(testCube, sizeof(testCube), &testCubeVAO, &testCubeVBO);
	unsigned int textVAO, textVBO;
	initTextVAO(&textVAO, &textVBO);

	//Geometry shaders
	Shader basicShader("src/Boilerplate/shaderBasicVertex.txt", "src/Boilerplate/shaderBasicFragment.txt");
	//Texture loading
	stbi_set_flip_vertically_on_load(true);
	basicShader.use(); 
	basicShader.setInt("texture1", 0);
	basicShader.setInt("texture2", 1);
	unsigned int texture1 = generateTexture("assets/textures/container.jpg", true);
	unsigned int texture2 = generateTexture("assets/textures/nice.jpg", true);

	//Text shaders
	Shader textShader("src/Boilerplate/shaderTextVertex.txt", "src/Boilerplate/shaderTextFragment.txt");
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(1440), 0.0f, static_cast<float>(1440));
	textShader.use();
	glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

	//Coordinate transformations
	//Transforms local coords to world coords
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//Transforms world coords to camera coords
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 projection; //Transforms camera coords to clip coords
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	// Time initialization
	Timer &timer = Timer::Instance();		// Create pointer to singleton timer instance
	std::shared_ptr<CallbackInterface> Callptr = processInput(window);

	int fps = 0;
	// PRIMARY GAME LOOP
	while (!glfwWindowShouldClose(window)) {
		// Update time
		timer.update();								// Update time instance
		double deltaTime = timer.getDeltaTime();	// Get delta time
		int fpsTest = timer.getFPS(0.2);			// Get fps (WARNING: can be NULL!)
		if (fpsTest != NULL) fps = fpsTest;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render cube
		// (will be encapsulated once ECS is in place)
		basicShader.use();
		view = glm::lookAt(Callptr->camera_pos, Callptr->camera_pos + Callptr->camera_front, Callptr->camera_up);
		model = glm::rotate(model, glm::radians(50.f) * (float)deltaTime, glm::vec3(0.5f, 1.0f, 0.0f));
		basicShader.setMat4("model", model);
		basicShader.setMat4("view", view);
		basicShader.setMat4("projection", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		glBindVertexArray(testCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Draw text
		RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 10.0f, 1390.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), Characters);
		
		// Swap buffers, poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Terminate program
	glfwTerminate();
	return 0;
}