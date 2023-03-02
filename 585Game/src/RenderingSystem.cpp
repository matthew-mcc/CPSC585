#include "RenderingSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/stb_image.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define _USE_MATH_DEFINES
#include <math.h>


// Rendering System Constructor
RenderingSystem::RenderingSystem() {
	initRenderer();
}

void RenderingSystem::SetupImgui() {
	// IMGUI INITIALIZATION
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// Initialize Renderer
void RenderingSystem::initRenderer() {
	// WINDOW INITIALIZATION
	window = initWindow();
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// FRAME BUFFER INITIALIZATIONS
	nearShadowMap = FBuffer(8192, 2048, 40.f, 10.f, -500.f, 100.f);
	farShadowMap = FBuffer(16384, 4096, 800.f, 300.f, -700.f, 1000.f);
	outlineMap = FBuffer(1920, 1080);
	outlineMapNoLandscape = FBuffer(1920, 1080);

	// WORLD SHADER INITIALIZATION
	stbi_set_flip_vertically_on_load(true);
	celShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");

	// FONT INITIALIZATION
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	initTextVAO(&textVAO, &textVBO);
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	textShader.use();

	// CAMERA POSITION
	camera_previous_position = vec3(1.0f);
}


// Update Renderer
void RenderingSystem::updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer) {
	// BACKGROUND
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);	// Set Background (Sky) Color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// IMGUI INITIALIZATION
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// CAMERA POSITION / LAG
	Entity playerEntity = gameState->findEntity("player_truck1");	// Find player entity

	// Retrieve player direction vectors
	vec3 player_forward = playerEntity.transform->getForwardVector();
	vec3 player_right = playerEntity.transform->getRightVector();
	vec3 player_up = playerEntity.transform->getUpVector();

	// Chase Camera: Compute eye and target offsets
		// Eye Offset: Camera position (world space)
		// Target Offset: Camera focus point (world space)
	vec3 eye_offset = (camera_position_forward * player_forward) + (camera_position_right * player_right) + (camera_position_up * player_up);
	vec3 target_offset = (camera_target_forward * player_forward) + (camera_target_right * player_right) + (camera_target_up * player_up);

	// Camera lag: Generate target_position - prev_position creating a vector. Scale by constant factor, then add to prev and update
	vec3 camera_target_position = playerEntity.transform->getPosition() + eye_offset;
	vec3 camera_track_vector = camera_target_position - camera_previous_position;
	camera_track_vector = camera_track_vector * camera_lag;
	camera_previous_position = vec3(translate(mat4(1.0f), camera_track_vector) * vec4(camera_previous_position, 1.0f));

	// If user is controlling camera, set view accordingly
	vec3 camOffset = vec3(0.f);
	if (callback_ptr->moveCamera) {
		camOffset = camera_previous_position - playerEntity.transform->getPosition();
		camOffset = vec4(camOffset, 0.f) * glm::rotate(glm::mat4(1.f), callback_ptr->xAngle, world_up);
		camOffset += playerEntity.transform->getPosition();
		view = lookAt(camOffset, playerEntity.transform->getPosition() + target_offset, world_up);
	}
	else {
		view = lookAt(camera_previous_position + camOffset, playerEntity.transform->getPosition() + target_offset, world_up);
	}

	// Set projection and view matrices
	projection = perspective(radians(fov), (float)callback_ptr->xres / (float)callback_ptr->yres, 0.1f, 1000.0f);

	// FIRST PASS: FAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 200.f;
	farShadowMap.update(lightPos, vec3(0.f));
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		// Retrieve global position and rotation
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			farShadowMap.shader.setMat4("model", model);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(farShadowMap.shader);
		}
	}
	glCullFace(GL_BACK);
	farShadowMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);


	// SECOND PASS: NEAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 40.f;
	nearShadowMap.update(lightPos, playerEntity.transform->getPosition());
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		// Retrieve global position and rotation
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			nearShadowMap.shader.setMat4("model", model);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(nearShadowMap.shader);
		}
	}
	glCullFace(GL_BACK);
	nearShadowMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);

	// THIRD PASS: TOON OUTLINE
	outlineMap.update(projection, view);
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		// Retrieve global position and rotation
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			outlineMap.shader.setMat4("model", model);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(outlineMap.shader);
		}
	}
	glCullFace(GL_BACK);
	outlineMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);

	// THIRD PASS: TOON OUTLINE 2 
	outlineMapNoLandscape.update(projection, view);
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		if (gameState->entityList.at(i).name.compare("landscape") == 0) continue;
		// Retrieve global position and rotation
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			outlineMapNoLandscape.shader.setMat4("model", model);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(outlineMapNoLandscape.shader);
		}
	}
	glCullFace(GL_BACK);
	outlineMapNoLandscape.cleanUp(callback_ptr);

	//farShadowMap.render();		//Uncomment to see the far shadow map (light's perspective, near the car)
	//nearShadowMap.render();		//Uncomment to see the near shadow map (light's perspective, the entire map)
	//outlineMap.render();			//Uncomment to see the outline map (camera's position, just a depth map)
	outlineMapNoLandscape.render();

	glClear(GL_DEPTH_BUFFER_BIT);

	// FOURTH PASS: CEL SHADE RENDER
		// Use world shader
	celShader.use();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, nearShadowMap.depthMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, farShadowMap.depthMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, outlineMap.depthMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, outlineMapNoLandscape.depthMap);
	setCelShaderUniforms();
	celShader.setBool("renderingLand", false);

	// Iteratively Draw Models
	for (int i = 0; i < gameState->entityList.size(); i++) {
		// Retrieve global position and rotation
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			/*if (gameState->entityList.at(i).name.compare("landscape") == 0) {
				celShader.setBool("renderingLand", true);
			}
			else celShader.setBool("renderingLand", false);*/

			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			celShader.setMat4("model", model);

			// Update relative light position
			quat lightRotation = rotation;
			quat localLightRotation = localRotation;
			lightRotation.w *= -1.f;
			localLightRotation *= -1.f;
			vec3 newLight = (vec3)(toMat4(lightRotation) * vec4(lightPos, 0.f) * toMat4(localLightRotation));
			celShader.setVec3("sun", newLight);

			// Draw model's mesh
			gameState->entityList.at(i).model->meshes.at(j).Draw(celShader);
		}
	}

	// FIFTH PASS: GUI RENDER
		// Use text shader
	textShader.use();
	mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptr->xres), 0.0f, static_cast<float>(callback_ptr->yres));
	textShader.setMat4("projection", textProjection);

	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.5);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 8.f, callback_ptr->yres - 32.f, 0.6f, vec3(0.2, 0.2f, 0.2f), textChars);

	// Display game timer / countdown
	if (timer->getCountdownSecs() >= 10) {
		RenderText(textShader, textVAO, textVBO, std::to_string(timer->getCountdownMins()) + ":" + std::to_string(timer->getCountdownSecs()),
			callback_ptr->xres / 2.f - 10.f,
			callback_ptr->yres - 32.f, 0.6f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);
	}
	else {
		RenderText(textShader, textVAO, textVBO, std::to_string(timer->getCountdownMins()) + ":0" + std::to_string(timer->getCountdownSecs()),
			callback_ptr->xres / 2.f - 10.f,
			callback_ptr->yres - 32.f, 0.6f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);
	}

	// Imgui Window
	ImGui::Begin("Super Space Salvagers - Debug Menu");
	ImGui::Text("Cel Shader Parameters");
	ImGui::SliderFloat("Light Rotation", &lightRotation, 0.f, 6.28f);
	ImGui::SliderFloat("Light Angle", &lightAngle, 0.f, 1.57f);
	ImGui::SliderFloat("Middle Band Width", &band, 0.f, 0.2f);
	ImGui::SliderFloat("Gradient Strength", &gradient, 0.0001f, 0.1f);
	ImGui::SliderFloat("Middle band shift", &shift, -0.5f, 0.5f);
	ImGui::ColorEdit3("Sky Color", (float*)&skyColor);
	ImGui::ColorEdit3("Highlight Color", (float*)&lightColor);
	ImGui::ColorEdit3("Shadow Color", (float*)&shadowColor);
	ImGui::ColorEdit3("Fog Color", (float*)&fogColor);
	ImGui::SliderFloat("Fog depth", &fogDepth, 0.f, 0.2f);
	ImGui::SliderFloat("Outline Transparency", &outlineTransparency, 0.f, 1.f);
	ImGui::SliderFloat("Outline Sensitivity", &outlineSensitivity, 0.f, 50.f);
	//ImGui::SliderFloat("Min bias", &minBias, 0.0f, 0.1f);
	ImGui::SliderFloat("Min bias", &minBias, 0.0f, 100.f);
	ImGui::SliderFloat("Max bias", &maxBias, 0.0f, 0.1f);

	ImGui::Text("Camera Parameters");
	ImGui::SliderFloat("Camera Position Forward", &camera_position_forward, -200.f, 200.f);
	ImGui::SliderFloat("Camera Position Up", &camera_position_up, -200.f, 200.f);
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
	celShader.setMat4("nearLightSpaceMatrix", nearShadowMap.lightSpaceMatrix);
	celShader.setMat4("farLightSpaceMatrix", farShadowMap.lightSpaceMatrix);
	celShader.setMat4("outlineSpaceMatrix", outlineMap.lightSpaceMatrix);
	celShader.setInt("nearShadowMap", 1);
	celShader.setInt("farShadowMap", 2);
	celShader.setInt("outlineMap", 3);
	celShader.setInt("outlineMapNoLandscape", 4);

	celShader.setVec3("lightColor", lightColor);
	celShader.setVec3("shadowColor", shadowColor);
	celShader.setVec3("fogColor", fogColor);
	celShader.setFloat("fogDepth", fogDepth);
	celShader.setVec3("sun", lightPos);
	celShader.setFloat("band", band);
	celShader.setFloat("gradient", gradient);
	celShader.setFloat("shift", shift);
	celShader.setFloat("minBias", minBias);
	celShader.setFloat("maxBias", maxBias);
	celShader.setFloat("outlineTransparency", outlineTransparency);
	celShader.setFloat("outlineSensitivity", outlineSensitivity);
}