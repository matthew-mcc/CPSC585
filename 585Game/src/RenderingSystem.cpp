#include "RenderingSystem.h"
#include "PhysicsSystem.h"

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
	outlineMap = FBuffer(1920, 1080, "o");
	outlineMapNoLandscape = FBuffer(1920, 1080, "o");
	celMap = FBuffer(1920, 1080, "c");
	blurMap = FBuffer(1920, 1080, "b");

	// WORLD SHADER INITIALIZATION
	stbi_set_flip_vertically_on_load(true);
	celShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");

	// FONT INITIALIZATION
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	initTextVAO(&textVAO, &textVBO);
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	textShader.use();
}

float timeTorset = 0.f;
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
	// Find player entity
	Entity* playerEntity = gameState->findEntity("vehicle_0");

	// Retrieve player direction vectors
	//camera_position_forward = camera_position_forward - (callback_ptr->camera_acceleration) / 15.f;
	vec3 player_forward = playerEntity->transform->getForwardVector();
	vec3 player_right = playerEntity->transform->getRightVector();
	vec3 player_up = playerEntity->transform->getUpVector();

	float camera_zoom_forward = clamp(1.0f + (float)playerEntity->nbChildEntities * 0.5f, 1.0f, 11.0f);
	float camera_zoom_up = clamp(1.0f + (float)playerEntity->nbChildEntities * 0.4f, 1.0f, 11.0f);


	


	// Chase Camera: Compute eye and target offsets
		// Eye Offset: Camera position (world space)
		// Target Offset: Camera focus point (world space)
	vec3 eye_offset = (camera_position_forward * player_forward * camera_zoom_forward) + (camera_position_right * player_right) + (camera_position_up * vec3(0.0f, 1.0f, 0.0f) * camera_zoom_up);
	vec3 target_offset = (camera_target_forward * player_forward) + (camera_target_right * player_right) + (camera_target_up * player_up);
	
	

	// Camera lag: Generate target_position - prev_position creating a vector. Scale by constant factor, then add to prev and update
	vec3 camera_target_position = playerEntity->transform->getPosition() + eye_offset;
	float y = playerEntity->transform->getPosition().y + camera_position_up + (float)playerEntity->nbChildEntities * 0.4f;
	camera_target_position.y = y;


	vec3 camera_track_vector = camera_target_position - camera_previous_position;

	camera_track_vector = camera_track_vector * camera_lag * (float)timer->getDeltaTime();
	
	camera_previous_position = vec3(translate(mat4(1.0f), camera_track_vector) * vec4(camera_previous_position, 1.0f));
	

	// If user is controlling camera, set view accordingly
	vec3 camOffset = vec3(0.f);
	if (callback_ptr->moveCamera) {
		camOffset = camera_previous_position - playerEntity->transform->getPosition();
		camOffset = vec4(camOffset, 0.f) * glm::rotate(glm::mat4(1.f), callback_ptr->xAngle, world_up);
		camOffset += playerEntity->transform->getPosition();
		view = lookAt(camOffset, playerEntity->transform->getPosition() + target_offset, world_up);
	}
	else {
		view = lookAt(camera_previous_position + camOffset, playerEntity->transform->getPosition() + target_offset, world_up);
	}
	
	glm::vec3 Camera_collision = PhysicsSystem::CameraRaycasting(camera_previous_position);
	if (Camera_collision.x != 0.f && Camera_collision.y != 0 && Camera_collision.z != 0) {//not sure which one to use XD
		camera_position_forward += Camera_collision.z;//* (float)timer->getDeltaTime();//camera_previous_position.z += 1.f; //* (float)timer->getDeltaTime();//cout << "???" << endl;
		camera_position_up += Camera_collision.y;
		camera_position_right -= Camera_collision.x;
		timeTorset = 0.f;
	}
	else {
		if (timeTorset > 1.f) { //lag to reset the camera
			float reset_speed = 5.f; // the speed to rset the camera
			if (camera_position_forward > -7.55f)
				camera_position_forward -= reset_speed * (float)timer->getDeltaTime();
			else if (camera_position_forward < -7.45f)
				camera_position_forward += reset_speed * (float)timer->getDeltaTime();

			if (camera_position_up > 3.55f)
				camera_position_up -= reset_speed * (float)timer->getDeltaTime();
			else if (camera_position_up < 3.45f)
				camera_position_up += reset_speed * (float)timer->getDeltaTime();

			if (camera_position_right > 0.05f)
				camera_position_right -= reset_speed * (float)timer->getDeltaTime();
			else if (camera_position_right < -0.05f)
				camera_position_right += reset_speed * (float)timer->getDeltaTime();
		}
		timeTorset += (float)timer->getDeltaTime();
	}

	// Set projection and view matrices
	projection = perspective(radians(fov), (float)callback_ptr->xres / (float)callback_ptr->yres, 0.1f, 10000.0f);
	

	// MESH ANIMATIONS
	// Center Portal
	Entity* portalEntity = gameState->findEntity("portal_center");
	vec3 rot(0.0f, 0.3f * timer->getDeltaTime(), 0.0f);
	portalEntity->localTransforms.at(0)->setRotation(normalize(portalEntity->localTransforms.at(0)->getRotation() * quat(rot)));


	// FIRST PASS: FAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 200.f;
	farShadowMap.update(lightPos, vec3(0.f));
	farShadowMap.render(gameState, "");
	farShadowMap.cleanUp(callback_ptr);

	// SECOND PASS: NEAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 40.f;
	nearShadowMap.update(lightPos, playerEntity->transform->getPosition());
	nearShadowMap.render(gameState, "");
	nearShadowMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);

	// THIRD PASS: TOON OUTLINE (Landscape)
	outlineMap.update(projection, view);
	outlineMap.render(gameState, "");
	outlineMap.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);

	// FOURTH PASS: TOON OUTLINE (Objects)
	outlineMapNoLandscape.update(projection, view);
	outlineMapNoLandscape.render(gameState, "l");
	outlineMapNoLandscape.cleanUp(callback_ptr);

	glClear(GL_DEPTH_BUFFER_BIT);

	// FIFTH PASS: SCENE TO TEXTURE
	celMap.debugShader.use();
	celMap.update(projection, view);
	glCullFace(GL_FRONT);
	bindTexture(1, nearShadowMap.fbTextures[0]);
	bindTexture(2, farShadowMap.fbTextures[0]);
	bindTexture(3, outlineMap.fbTextures[0]);
	bindTexture(4, outlineMapNoLandscape.fbTextures[0]);
  
	setCelShaderUniforms(&celMap.shader);
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
			celShader.setMat4("model", model);

			// Update relative light position
			quat lightRotation = rotation;
			quat localLightRotation = localRotation;
			lightRotation.w *= -1.f;
			localLightRotation *= -1.f;
			vec3 newLight = (vec3)(toMat4(lightRotation) * vec4(lightPos, 0.f) * toMat4(localLightRotation));
			celShader.setVec3("sun", newLight);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(celMap.shader);
		}
	}
  
	glCullFace(GL_BACK);
	celMap.cleanUp(callback_ptr);

	//farShadowMap.renderToScreen();		//Uncomment to see the far shadow map (light's perspective, near the car)
	//nearShadowMap.renderToScreen();		//Uncomment to see the near shadow map (light's perspective, the entire map)
	//outlineMap.renderToScreen();			//Uncomment to see the outline map (camera's position, just a depth map)
	//outlineMapNoLandscape.renderToScreen();

	// BLUR IMAGE FOR BLOOM
	if (blurMap.getWidth() != callback_ptr->xres || blurMap.getHeight() != callback_ptr->yres) {
		blurMap = FBuffer(callback_ptr->xres, callback_ptr->yres, "b");
	}

	bool horizontal = true, first_iteration = true;
	int amount = 10;
	blurMap.shader.use();
	blurMap.shader.setInt("image", 1);
	for (unsigned int i = 0; i < amount; i++)
	{
		blurMap.shader.use();
		glBindFramebuffer(GL_FRAMEBUFFER, blurMap.FBO[horizontal]);
		blurMap.shader.setInt("horizontal", horizontal);
		bindTexture(1, first_iteration ? celMap.fbTextures[1] : blurMap.fbTextures[!horizontal]);
		blurMap.renderQuad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	bindTexture(1, celMap.fbTextures[0]);
	bindTexture(2, blurMap.fbTextures[0]);

	celMap.renderToScreen();
	//blurMap.renderToScreen();

	// SIXTH PASS: GUI RENDER
		// Use text shader
	textShader.use();
	mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptr->xres), 0.0f, static_cast<float>(callback_ptr->yres));
	textShader.setMat4("projection", textProjection);

	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.5);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 8.f, callback_ptr->yres - 32.f, 0.6f, vec3(0.2, 0.2f, 0.2f), textChars);

	// Game Ended Screen
	if (gameState->gameEnded) {
		string winnerText;
		if (gameState->winner == NULL) {
			winnerText = "Tie Game!";
		}
		else {
			string winnerName = gameState->winner->name;
			if (winnerName == "vehicle_0") winnerText = "Salvager #1 Wins!";
			if (winnerName == "vehicle_1") winnerText = "Salvager #2 Wins!";
			if (winnerName == "vehicle_2") winnerText = "Salvager #3 Wins!";
			if (winnerName == "vehicle_3") winnerText = "Salvager #4 Wins!";
		}
		RenderText(textShader, textVAO, textVBO, winnerText,
			callback_ptr->xres / 2 - (18 * winnerText.length()),
			callback_ptr->yres / 2 + 150,
			1.5f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);
	}

	// Normal Gameplay Screen
	else {
		// Display game timer / countdown
		std::string timerMins = std::to_string(abs(timer->getCountdownMins()));
		std::string timerSeconds = std::to_string(abs(timer->getCountdownSecs()));
		std::string overtime = "Overtime: ";
		std::string zero = "0";
		float timer_xoffset = callback_ptr->xres / 2.f - 10.f;

		if (timerSeconds.size() < 2) {
			timerSeconds.insert(0, zero);
		}
		if (timer->getCountdown() < 0) {
			timerMins.insert(0, overtime);
			timer_xoffset = callback_ptr->xres / 2.f - 100.f;
		}

		RenderText(textShader, textVAO, textVBO, timerMins + ":" + timerSeconds,
			timer_xoffset,
			callback_ptr->yres - 32.f, 0.6f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);

		// Display boost meter
		RenderText(textShader, textVAO, textVBO, "Boost Meter: " + to_string((int)playerEntity->playerProperties->boost_meter),
			20,
			40, 0.6f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);

		// Display player scores
		string scoreText = "";
		if (gameState->findEntity("vehicle_0") != NULL) {
			scoreText = "Salvager #1: " + to_string(gameState->findEntity("vehicle_0")->playerProperties->getScore());
			RenderText(textShader, textVAO, textVBO, scoreText,
				callback_ptr->xres - (16 * (int)scoreText.size()),
				callback_ptr->yres - 100.f,
				0.6f,
				vec3(0.2, 0.2f, 0.2f),
				textChars);
		}
		if (gameState->findEntity("vehicle_1") != NULL) {
			scoreText = "Salvager #2: " + to_string(gameState->findEntity("vehicle_1")->playerProperties->getScore());
			RenderText(textShader, textVAO, textVBO, scoreText,
				callback_ptr->xres - (16 * (int)scoreText.size()),
				callback_ptr->yres - 150.f,
				0.6f,
				vec3(0.2, 0.2f, 0.2f),
				textChars);
		}
		if (gameState->findEntity("vehicle_2") != NULL) {
			scoreText = "Salvager #3: " + to_string(gameState->findEntity("vehicle_2")->playerProperties->getScore());
			RenderText(textShader, textVAO, textVBO, scoreText,
				callback_ptr->xres - (16 * (int)scoreText.size()),
				callback_ptr->yres - 200.f,
				0.6f,
				vec3(0.2, 0.2f, 0.2f),
				textChars);
		}
		if (gameState->findEntity("vehicle_3") != NULL) {
			scoreText = "Salvager #4: " + to_string(gameState->findEntity("vehicle_3")->playerProperties->getScore());
			RenderText(textShader, textVAO, textVBO, scoreText,
				callback_ptr->xres - (16 * (int)scoreText.size()),
				callback_ptr->yres - 250.f,
				0.6f,
				vec3(0.2, 0.2f, 0.2f),
				textChars);
		}
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


void RenderingSystem::setCelShaderUniforms(Shader* shader) {
	(*shader).setMat4("model", model);
	(*shader).setMat4("view", view);
	(*shader).setMat4("projection", projection);
	(*shader).setMat4("nearLightSpaceMatrix", nearShadowMap.lightSpaceMatrix);
	(*shader).setMat4("farLightSpaceMatrix", farShadowMap.lightSpaceMatrix);
	(*shader).setMat4("outlineSpaceMatrix", outlineMap.lightSpaceMatrix);
	(*shader).setInt("nearShadowMap", 1);
	(*shader).setInt("farShadowMap", 2);
	(*shader).setInt("outlineMap", 3);
	(*shader).setInt("outlineMapNoLandscape", 4);

	(*shader).setVec3("lightColor", lightColor);
	(*shader).setVec3("shadowColor", shadowColor);
	(*shader).setVec3("fogColor", fogColor);
	(*shader).setFloat("fogDepth", fogDepth);
	(*shader).setVec3("sun", lightPos);
	(*shader).setFloat("band", band);
	(*shader).setFloat("gradient", gradient);
	(*shader).setFloat("shift", shift);
	(*shader).setFloat("minBias", minBias);
	(*shader).setFloat("maxBias", maxBias);
	(*shader).setFloat("outlineTransparency", outlineTransparency);
	(*shader).setFloat("outlineSensitivity", outlineSensitivity);
}

void RenderingSystem::bindTexture(int location, unsigned int texture) {
	glActiveTexture(GL_TEXTURE0 + location);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE0);
}