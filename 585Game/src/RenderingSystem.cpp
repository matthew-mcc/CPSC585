#include "RenderingSystem.h"
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
	worldShader = Shader("C:/GIT/CPSC585/585Game/src/Boilerplate/shaderBasicVertex.txt", "C:/GIT/CPSC585/585Game/src/Boilerplate/shaderBasicFragment.txt");
	stbi_set_flip_vertically_on_load(true);
	worldShader.use();

// INTIAL MODEL, VIEW, AND PROJECTION MATRICES
	// Coordinate transformations
	model = glm::mat4(1.0f);																	// Transforms local coords to world coords
	view = glm::mat4(1.0f);																		// Transforms world coords to camera coords
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);			// Transforms camera coords to clip coords

// FONT INITIALIZATION
	// Create character map based on font file
	textChars = initFont("C:/GIT/CPSC585/585Game/assets/fonts/arial.ttf");
	// Create vertex array and buffer objects
	initTextVAO(&textVAO, &textVBO);
	// Create text shader
	textShader = Shader("C:/GIT/CPSC585/585Game/src/Boilerplate/shaderTextVertex.txt", "C:/GIT/CPSC585/585Game/src/Boilerplate/shaderTextFragment.txt");
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(1440), 0.0f, static_cast<float>(1440));
	textShader.use();
	glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));
}


// Update Renderer
void RenderingSystem::updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr, std::vector<Entity> entityList) {

// TIME UPDATE
	// Get deltaTime Update
	timer->update();								// Update time instance
	double deltaTime = timer->getDeltaTime();		// Get delta time

// WORLD SHADER
	// Use world shader, set scene background colour
	worldShader.use();
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TEMP: Simple camera look
	view = glm::lookAt(callback_ptr->camera_pos, callback_ptr->camera_pos + callback_ptr->camera_front, callback_ptr->camera_up);

	// Set MVP matrices
	//worldShader.setMat4("model", model);
	worldShader.setMat4("view", view);
	worldShader.setMat4("projection", projection);

	// Draw models
	for (int i = 0; i < entityList.size(); i++) {
		glm::vec3 position = entityList.at(i).transform->position;
		glm::quat rotation = entityList.at(i).transform->rotation;

		//std::cout << entityList.at(i).transform->position.x << " , " << entityList.at(i).transform->position.y << "\n";


		model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = model * glm::toMat4(rotation);
		model = glm::scale(model, glm::vec3(1.0f));
		worldShader.setMat4("model", model);
		entityList.at(i).model->Draw(worldShader);
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