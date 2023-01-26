#include "RenderingSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

#include <Boilerplate/stb_image.h>


// Rendering System Constructor
RenderingSystem::RenderingSystem() {
	initRenderer();
}


// Initialize Renderer
void RenderingSystem::initRenderer() {
// WINDOW INITIALIZATION
	window = initWindow();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// TIMER INITIALIZATION
	timer = &Timer::Instance();		// Create pointer to singleton timer instance
	
// WORLD SHADER INITIALIZATION
	// Bind vertex and fragment shaders to world shader object
	worldShader = Shader("src/Boilerplate/shaderBasicVertex.txt", "src/Boilerplate/shaderBasicFragment.txt");
	stbi_set_flip_vertically_on_load(true);
	worldShader.use();

// INTIAL MODEL, VIEW, AND PROJECTION MATRICES
	// Coordinate transformations
	model = glm::mat4(1.0f);																	// Transforms local coords to world coords
	view = glm::mat4(1.0f);																		// Transforms world coords to camera coords
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);			// Transforms camera coords to clip coords

	outlineShader = Shader("src/Cel Shader/shaderOutlineVertex.txt", "src/Cel Shader/shaderOutlineFragment.txt");

// FONT INITIALIZATION
	// Create character map based on font file
	textChars = initFont("assets/fonts/arial.ttf");
	// Create vertex array and buffer objects
	initTextVAO(&textVAO, &textVBO);
	// Create text shader
	textShader = Shader("src/Boilerplate/shaderTextVertex.txt", "src/Boilerplate/shaderTextFragment.txt");
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(1440), 0.0f, static_cast<float>(1440));
	textShader.use();
	glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

// MODEL INITIALIZATION
	Model testModel = Model("assets/models/tire1/tire1.obj");
	models.push_back(testModel);
}


// Update Renderer
void RenderingSystem::updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr) {

// TIME UPDATE
	// Get deltaTime Update
	timer->update();								// Update time instance
	double deltaTime = timer->getDeltaTime();		// Get delta time

// WORLD SHADER
	// Use world shader, set scene background colour
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TEMP: Simple camera look
	view = glm::lookAt(callback_ptr->camera_pos, callback_ptr->camera_pos + callback_ptr->camera_front, callback_ptr->camera_up);


	// Draw outlines
	outlineShader.use();
	outlineShader.setMat4("model", model);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	outlineShader.setFloat("thickness", 0.2f);
	for (int i = 0; i < models.size(); i ++) {
		models.at(i).Draw(outlineShader);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	//Draw models
	worldShader.use();
	worldShader.setMat4("model", model);
	worldShader.setMat4("view", view);
	worldShader.setMat4("projection", projection);
	for (int i = 0; i < models.size(); i ++) {
		models.at(i).Draw(worldShader);
	}

// TEXT SHADER
	// Use text shader
	textShader.use();
	
	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.2);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 10.0f, 1390.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), textChars);

// PREPARE FOR NEXT FRAME
	// Swap buffers, poll events
	glfwSwapBuffers(window);
	glfwPollEvents();


}