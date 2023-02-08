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
	glEnable(GL_MULTISAMPLE);
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
	
// SHADOW MAP INITIALIZATION
	//shadowMap = Shadow(16384, 4096);
	nearShadowMap = Shadow(8192, 2048, 40.f, 10.f, 0.1f, 100.f);
	farShadowMap = Shadow(2048, 512, 250.f, 75.f, 0.1f, 500.f);

// WORLD SHADER INITIALIZATION
	// Bind vertex and fragment shaders to world shader object
	stbi_set_flip_vertically_on_load(true);
	celShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");
	outlineShader = Shader("src/Shaders/outlineVertex.txt", "src/Shaders/outlineFragment.txt");

// FONT INITIALIZATION
	// Create character map based on font file
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	// Create vertex array and buffer objects
	initTextVAO(&textVAO, &textVBO);
	// Create text shader
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	textShader.use();
}

// Update Renderer
void RenderingSystem::updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer) {

// BACKGROUND
	// Set Background (Sky) Color
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// IMGUI
	// Create IMGUI Window
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Draw outlines
	/*outlineShader.use();
	outlineShader.setMat4("model", model);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	outlineShader.setFloat("thickness", 0.2f);
	for (int i = 0; i < models.size(); i ++) {
		models.at(i).Draw(outlineShader);
	}*/

// FIRST PASS: FAR SHADOWMAP RENDER
	/*lightPos = glm::vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 200.f;
	farShadowMap.update(lightPos);
	glCullFace(GL_FRONT);
	for (int i = 0; i < models.size(); i++) {
		if (i == models.size() - 1) {
			farShadowMap.shader.setMat4("model", glm::translate(model, glm::vec3(0.f, altitude, 0.f)));
		}
		models.at(i).Draw(farShadowMap.shader);
	}
	glCullFace(GL_BACK);
	farShadowMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);*/

// SECOND PASS: NEAR SHADOWMAP RENDER
	lightPos = glm::vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 40.f;
	nearShadowMap.update(lightPos);
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		if (gameState->entityList.at(i).name == "landscape") {
			nearShadowMap.shader.setMat4("model", glm::translate(model, glm::vec3(0.f, altitude, 0.f)));
		}
		gameState->entityList.at(i).model->Draw(nearShadowMap.shader);
	}
	glCullFace(GL_BACK);
	nearShadowMap.cleanUp(callback_ptr);

	//farShadowMap.render();		//Uncomment to see the shadow map (scene rendered from light's point of view)
	//nearShadowMap.render();		//Uncomment to see the shadow map (scene rendered from light's point of view)
	
	glClear(GL_DEPTH_BUFFER_BIT);

// THIRD PASS: CEL SHADE RENDER
	// Use world shader
	celShader.use();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, nearShadowMap.depthMap);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, farShadowMap.depthMap);

	// Retrieve player entity
	Entity playerEntity = gameState->findEntity("player_truck1");

	camera_position_forward = camera_position_default-(callback_ptr->camera_acceleration)/15.f;
	glm::vec3 player_forward = playerEntity.transform->getForwardVector();
	glm::vec3 player_right = playerEntity.transform->getRightVector();
	glm::vec3 player_up = playerEntity.transform->getUpVector();

	// Chase Camera: Compute eye and target offsets for lookat view matrix
	glm::vec3 eye_offset = (camera_position_forward * player_forward) + (callback_ptr->camera_position_right * player_right) + (camera_position_up * player_up);
	glm::vec3 target_offset = (camera_target_forward * player_forward) + (camera_target_right * player_right) + (camera_target_up * player_up);
	
	// Set view and projection matrices
	view = glm::lookAt(playerEntity.transform->getPosition() + eye_offset, playerEntity.transform->getPosition() + target_offset, callback_ptr->camera_up);
	projection = glm::perspective(glm::radians(callback_ptr->fov), (float)callback_ptr->xres / (float)callback_ptr->yres, 0.1f, 1000.0f);
	setCelShaderUniforms();

	// Iteratively Draw Models
	for (int i = 0; i < gameState->entityList.size(); i ++) {
		// Retrieve position and rotation
		glm::vec3 position = gameState->entityList.at(i).transform->getPosition();
		glm::quat rotation = gameState->entityList.at(i).transform->getRotation();

		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			glm::vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			glm::quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = glm::mat4(1.0f);
			model = glm::translate(model, position);
			model = model * glm::toMat4(rotation);
			model = glm::translate(model, localPosition);
			model = model * glm::toMat4(localRotation);
			model = scale(model, glm::vec3(1.0f));
			celShader.setMat4("model", model);

			gameState->entityList.at(i).model->meshes.at(j).Draw(celShader);
		}
	}

// FOURTH PASS: GUI RENDER
	// Use text shader
	textShader.use();
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(callback_ptr->xres), 0.0f, static_cast<float>(callback_ptr->yres));
	textShader.setMat4("projection", textProjection);

	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.5);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 8.f, callback_ptr->yres - 32.f, 0.6f, glm::vec3(0.2, 0.2f, 0.2f), textChars);

	int countdownMins = timer->getCountdown();
	countdownMins = countdownMins / 60;
	int countdownSeconds = timer->getCountdown();
	countdownSeconds = countdownSeconds % 60;
	
	if (countdownSeconds > 10) {
		RenderText(textShader, textVAO, textVBO, std::to_string(countdownMins) + ":" + std::to_string(countdownSeconds), callback_ptr->xres / 2.f - 10.f, callback_ptr->yres - 32.f, 0.6f, glm::vec3(0.2, 0.2f, 0.2f), textChars);
	}
	else {
		RenderText(textShader, textVAO, textVBO, std::to_string(countdownMins) + ":0" + std::to_string(countdownSeconds), callback_ptr->xres / 2.f - 10.f, callback_ptr->yres - 32.f, 0.6f, glm::vec3(0.2, 0.2f, 0.2f), textChars);

	}

	// Imgui Window
	ImGui::Begin("Super Space Salvagers - Debug");
	//ImGui::Text("World Parameters");
	//ImGui::SliderFloat("Landscape Altitude", &altitude, 0.f, 10.f);

	ImGui::Text("Cel Shader Parameters");
	ImGui::SliderFloat("Light Rotation", &lightRotation, 0.f, 6.28f);
	ImGui::SliderFloat("Light Angle", &lightAngle, 0.f, 1.57f);
	ImGui::SliderFloat("Middle Band Width", &band, 0.f, 0.2f);
	ImGui::SliderFloat("Gradient Strength", &gradient, 0.0001f, 0.1f);
	ImGui::SliderFloat("Middle band shift", &shift, -0.5f, 0.5f);
	ImGui::ColorEdit3("Sky Color", (float*)&skyColor);
	ImGui::ColorEdit3("Highlight Color", (float*)&lightColor);
	ImGui::ColorEdit3("Shadow Color", (float*)&shadowColor);
	ImGui::SliderFloat("Min bias", &minBias, 0.0f, 0.1f);
	ImGui::SliderFloat("Max bias", &maxBias, 0.0f, 0.1f);

	ImGui::Text("Camera Parameters");
	ImGui::SliderFloat("Camera Position Forward", &camera_position_default, -30.f, 30.f);
	ImGui::SliderFloat("Camera Position Up", &camera_position_up, -30.f, 30.f);
	ImGui::SliderFloat("Camera Position Right", &camera_position_right, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Forward", &camera_target_forward, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Up", &camera_target_up, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Right", &camera_target_right, -30.f, 30.f);
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
	celShader.setMat4("model", model);
	celShader.setMat4("view", view);
	celShader.setMat4("projection", projection);
	celShader.setMat4("lightSpaceMatrix", nearShadowMap.lightSpaceMatrix);
	celShader.setInt("nearShadowMap", 1);
	celShader.setInt("farShadowMap", 2);

	celShader.setVec3("lightColor", lightColor);
	celShader.setVec3("shadowColor", shadowColor);
	celShader.setVec3("sun", lightPos);
	celShader.setFloat("band", band);
	celShader.setFloat("gradient", gradient);
	celShader.setFloat("shift", shift);
	celShader.setFloat("minBias", minBias);
	celShader.setFloat("maxBias", maxBias);
}