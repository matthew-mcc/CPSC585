#include "RenderingSystem.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/stb_image.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

// IMGUI INITIALIZATION
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

// TIMER INITIALIZATION
	timer = &Timer::Instance();		// Create pointer to singleton timer instance
	
// INTIAL MODEL, VIEW, AND PROJECTION MATRICES
	// Coordinate transformations
	model = glm::mat4(1.0f);																	// Transforms local coords to world coords
	view = glm::mat4(1.0f);																		// Transforms world coords to camera coords
	projection = glm::perspective(glm::radians(45.0f), 1440.0f / 1440.0f, 0.1f, 1000.0f);		// Transforms camera coords to clip coords

// SHADOW MAP INITIALIZATION
	shadowMap = Shadow(4096, 4096);

// WORLD SHADER INITIALIZATION
	// Bind vertex and fragment shaders to world shader object
	stbi_set_flip_vertically_on_load(true);
	worldShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");
	outlineShader = Shader("src/Shaders/outlineVertex.txt", "src/Shaders/outlineFragment.txt");

// FONT INITIALIZATION
	// Create character map based on font file
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	// Create vertex array and buffer objects
	initTextVAO(&textVAO, &textVBO);
	// Create text shader
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(1440), 0.0f, static_cast<float>(1440));
	textShader.use();
	textShader.setMat4("projection", textProjection);

// MODEL INITIALIZATION
	Model testModel = Model("assets/models/test_truck1/test_truck1.obj");
	models.push_back(testModel);
	testModel = Model("assets/models/test_tire1/test_tire1.obj");
	models.push_back(testModel);
	testModel = Model("assets/models/landscape1/landscape1.obj");
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
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create IMGUI Window
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// TEMP: Simple camera look
	view = glm::lookAt(callback_ptr->camera_pos, callback_ptr->camera_pos + callback_ptr->camera_front, callback_ptr->camera_up);


	// Draw outlines
	/*outlineShader.use();
	outlineShader.setMat4("model", model);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	outlineShader.setFloat("thickness", 0.2f);
	for (int i = 0; i < models.size(); i ++) {
		models.at(i).Draw(outlineShader);
	}*/

	// FIRST PASS: SHADOWMAP RENDER
	lightPos = glm::vec3(sin(lightRotation), 0.5f, cos(lightRotation)) * 4.f;
	shadowMap.update(lightPos);
	glCullFace(GL_FRONT);
	for (int i = 0; i < models.size(); i++) {
		models.at(i).Draw(shadowMap.shader);
	}
	glCullFace(GL_BACK);
	shadowMap.cleanUp();
	//shadowMap.render();		//Uncomment to see the shadow map (scene rendered from light's point of view)

	glClear(GL_DEPTH_BUFFER_BIT);

	// SECOND PASS: FINAL MODEL RENDER
	worldShader.use();
	setCelShaderUniforms();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap.depthMap);
	for (int i = 0; i < models.size(); i ++) {
		models.at(i).Draw(worldShader);
	}

// TEXT SHADER
	// Use text shader
	textShader.use();
	
	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.2);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 10.0f, 1015.0f, 1.0f, glm::vec3(0.95, 0.95f, 0.95f), textChars);
	//glm::vec3(0.5, 0.8f, 0.2f)
// IMGUI WINDOW
	ImGui::Begin("Super Space Salvagers");
	ImGui::Text("Cel Shader Parameters");
	ImGui::SliderFloat("Light Angle", &lightRotation, 0.f, 6.f);
	ImGui::SliderFloat("Middle Band Width", &band, 0.f, 0.2f);
	ImGui::SliderFloat("Gradient Strength", &gradient, 0.0001f, 0.1f);
	ImGui::SliderFloat("Middle band shift", &shift, -0.5f, 0.5f);
	ImGui::ColorEdit3("Sky Color", (float*)&skyColor);
	ImGui::ColorEdit3("Highlight Color", (float*)&lightColor);
	ImGui::ColorEdit3("Shadow Color", (float*)&shadowColor);
	ImGui::SliderFloat("Min bias", &minBias, 0.0f, 0.1f);
	ImGui::SliderFloat("Max bias", &maxBias, 0.0f, 0.1f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// PREPARE FOR NEXT FRAME
	// Swap buffers, poll events
	glfwSwapBuffers(window);
	glfwPollEvents();


}

void RenderingSystem::shutdownImgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void RenderingSystem::setCelShaderUniforms() {
	worldShader.setMat4("model", model);
	worldShader.setMat4("view", view);
	worldShader.setMat4("projection", projection);
	worldShader.setMat4("lightSpaceMatrix", shadowMap.lightSpaceMatrix);
	worldShader.setInt("shadowMap", 1);

	worldShader.setVec3("lightColor", lightColor);
	worldShader.setVec3("shadowColor", shadowColor);
	worldShader.setVec3("sun", lightPos);
	worldShader.setFloat("band", band);
	worldShader.setFloat("gradient", gradient);
	worldShader.setFloat("shift", shift);
	worldShader.setFloat("minBias", minBias);
	worldShader.setFloat("maxBias", maxBias);
}