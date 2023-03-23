#include "RenderingSystem.h"
#include "PhysicsSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/stb_image.h>
#include <Boilerplate/Texture.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define _USE_MATH_DEFINES
#include <math.h>


// Rendering System Constructor
RenderingSystem::RenderingSystem() {
	initRenderer();
}

void RenderingSystem::resetRenderer() {
	camera_previous_position = vec3(0.0f, 8.0f, -270.0f);
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

	testTexture = generateTexture("assets/textures/alien.png", false);
	orbTexture = generateTexture("assets/textures/orb.png", false);

	// Choose one, top = squares, bottom = chevrons
	boostBlue = generateTexture("assets/textures/UI/boostBlue.png", false);
	boostGrey = generateTexture("assets/textures/UI/boostGrey.png", false);
	boostOrange = generateTexture("assets/textures/UI/boostOrange.png", false);
	// Will default to chevrons if not commented out
	boostBlue = generateTexture("assets/textures/UI/boostChevBlue.png", false);
	boostGrey = generateTexture("assets/textures/UI/boostChevGrey.png", false);
	boostOrange = generateTexture("assets/textures/UI/boostChevOrange.png", false);

	boostOn = generateTexture("assets/textures/UI/dotsOn.png", false);
	boostOff = generateTexture("assets/textures/UI/dotsOff.png", false);

	podcounterOn = generateTexture("assets/textures/UI/podcounterOn.png", false);
	podcounterOff = generateTexture("assets/textures/UI/podCounterOff.png", false);

	menuBackground = generateTexture("assets/textures/UI/menuBackground.png", false);
	menuLoading = generateTexture("assets/textures/UI/menuLoading.png", false);

	for (int i = 1; i <= 6; i++) {
		const std::string path = "assets/textures/UI/" + to_string(i) + ".png";
		ui_player_tracker.push_back(generateTexture(path.c_str(), false));
	}






	// PARTICLE SYSTEM INITIALIZATIONS
	particleShader = Shader("src/Shaders/particleVertex.txt", "src/Shaders/particleFragment.txt");
	particleShader.use();
	particleShader.setInt("sprite", 1);
	testParticles = ParticleSystem(particleShader, orbTexture, 400);

	// FRAME BUFFER INITIALIZATIONS
	nearShadowMap = FBuffer(8192, 2048, 40.f, 10.f, -500.f, 100.f);
	farShadowMap = FBuffer(16384, 4096, 800.f, 300.f, -700.f, 1000.f);
	outlineMap = FBuffer(1920, 1080, "o");
	outlineMapNoLandscape = FBuffer(1920, 1080, "o");
	outlineToTexture = FBuffer(1920, 1080, "ot");
	celMap = FBuffer(1920, 1080, "c");
	blurMap = FBuffer(1920, 1080, "b");
	intermediateBuffer = FBuffer(1920, 1080, "i");

	// WORLD SHADER INITIALIZATION
	stbi_set_flip_vertically_on_load(true);
	celShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");

	// FONT INITIALIZATION
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	initTextVAO(&textVAO, &textVBO);
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	textShader.use();
}

void resetValue(float &target, float range, float desireValue,float speed,float step) {
	if (target > desireValue)
		target -= speed * step;
	else if(target < desireValue)
		target += speed * step;
	
	if (target > (desireValue - range) && target < (desireValue + range))
		target = desireValue;
}
void RenderingSystem::updateRadius(float base, float zoom) {
	camera_radius = base + zoom;
}

// Update Renderer
void RenderingSystem::updateRenderer(std::shared_ptr<CallbackInterface> cbp, GameState* gameState, Timer* timer) {
	// CALLBACK POINTER
	callback_ptr = cbp;

	// BACKGROUND
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);	// Set Background (Sky) Color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gameState->gameEnded && !callback_ptr->play) {
		gameState->inMenu = true;
		gameState->gameEnded = false;
	}

	// MAIN MENU
	if (gameState->inMenu) {
		// Use Text Shader
		textShader.use();
		mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptr->xres), 0.0f, static_cast<float>(callback_ptr->yres));
		textShader.setMat4("projection", textProjection);

		// Load Screen
		if (callback_ptr->play) {
			drawUI(menuLoading, 0, 0, callback_ptr->xres, callback_ptr->yres, 0);
			gameState->inMenu = false;
		}

		// Menu Screen
		else {
		}

		drawUI(menuBackground, 0, 0, callback_ptr->xres, callback_ptr->yres, 1);
		glfwSwapBuffers(window);
		glfwPollEvents();
		return;
	}

	// IMGUI INITIALIZATION
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// CAMERA POSITION / LAG
	// Retrieve player direction vectors
	Entity* playerEntity = gameState->findEntity("vehicle_0");
	vec3 player_forward = playerEntity->transform->getForwardVector();
	vec3 player_right = playerEntity->transform->getRightVector();
	vec3 player_up = playerEntity->transform->getUpVector();
	vec3 player_pos = playerEntity->transform->getPosition();

	// Camera Zoom: Pull view back with increasing number of towed trailers
	float camera_zoom_forward = clamp(1.0f + (float)playerEntity->nbChildEntities * 0.5f, 1.0f, 11.0f);
	float camera_zoom_up = clamp(1.0f + (float)playerEntity->nbChildEntities * 0.4f, 1.0f, 11.0f);
	
	// Chase Camera: Compute eye and target offsets
		// Eye Offset: Camera position (world space)
		// Target Offset: Camera focus point (world space)
	vec3 eye_offset = (camera_position_forward * player_forward * camera_zoom_forward) + (camera_position_right * player_right) + (camera_position_up * player_up * camera_zoom_up);
	vec3 target_offset = player_pos + (camera_target_forward * player_forward) + (camera_target_right * player_right) + (camera_target_up * player_up);

	// If user is controlling camera, set view accordingly
	vec3 ResetVec = playerEntity->transform->getRotation()*vec3(0.f,3.5f*camera_zoom_up,-7.5f*camera_zoom_forward);
	glm::vec3 Camera_collision(0.f);
	glm::vec3 Reset_collision(1.f);

	// Camera Look: Orbit camera around vehicle
	vec3 lookOffset = player_pos + eye_offset;
	float lag_amount = camera_lag;
	if (callback_ptr->moveCamera) {
		lookOffset = vec4(eye_offset, 0.f) * glm::rotate(glm::mat4(1.f), callback_ptr->xAngle, world_up);
		lookOffset += player_pos;
		lag_amount = camera_lag * 4.f;
		Camera_collision = PhysicsSystem::CameraRaycasting(camera_previous_position,camera_radius,1.f);
	}
	else {
		Camera_collision = PhysicsSystem::CameraRaycasting(camera_previous_position,camera_radius,1.f);
		Reset_collision = PhysicsSystem::CameraRaycasting(player_pos+ResetVec, camera_radius, 1.f);
	}

	// Camera lag: Generate target_position - prev_position creating a vector. Scale by constant factor, then add to prev and update
	vec3 camera_target_position = lookOffset;
	camera_target_position.y = player_pos.y + camera_position_up + (float)playerEntity->nbChildEntities * 0.4f;
	vec3 camera_track_vector = camera_target_position - camera_previous_position;
	camera_track_vector = camera_track_vector * lag_amount * (float)timer->getDeltaTime();
	camera_previous_position = vec3(translate(mat4(1.0f), camera_track_vector) * vec4(camera_previous_position, 1.0f));

	// Set view matrix
	view = lookAt(camera_previous_position, target_offset, world_up);

	// For audio - probably need to change later
	gameState->listener_position = camera_previous_position;

	if (Camera_collision.x != 0.f && Camera_collision.y != 0 && Camera_collision.z != 0) {//not sure which one to use XD
		camera_position_forward += Camera_collision.z;//* (float)timer->getDeltaTime();//camera_previous_position.z += 1.f; //* (float)timer->getDeltaTime();//cout << "???" << endl;
		camera_position_up = clamp(camera_position_up+Camera_collision.y,3.5f,100.f);
		camera_position_right -= Camera_collision.x;
		rad_base = clamp(rad_base - sqrtf(Camera_collision.z * Camera_collision.z + Camera_collision.x * Camera_collision.x),0.5f,7.5f);
		//timeTorset = 0.f;
	}
	else {
		if (Reset_collision.x == 0.f && Reset_collision.y == 0 && Reset_collision.z == 0 && !PhysicsSystem::CameraIntercetionRaycasting(player_pos + ResetVec)) { //lag to reset the camera
			float reset_speed = 50.f; // the speed to rset the camera
			float step_time = (float)timer->getDeltaTime();
			float step_range = step_time * reset_speed;
			resetValue(camera_position_forward,step_range,-7.5f,reset_speed, step_time);
			resetValue(camera_position_up, step_range, 3.5f, reset_speed, step_time);
			resetValue(camera_position_right, step_range, 0.f, reset_speed, step_time);
			resetValue(rad_base, step_range, 7.5f, reset_speed, step_time);
		}
		//timeTorset += (float)timer->getDeltaTime();
	}
	updateRadius(rad_base,camera_zoom_forward);
	// Set projection and view matrices
	projection = perspective(radians(fov), (float)callback_ptr->xres / (float)callback_ptr->yres, 0.1f, 10000.0f);
	

	// MESH ANIMATIONS
	// Center Portal
	Entity* portalEntity = gameState->findEntity("portal_center");
	vec3 rot(0.0f, 0.3f * timer->getDeltaTime(), 0.0f);
	portalEntity->localTransforms.at(0)->setRotation(normalize(portalEntity->localTransforms.at(0)->getRotation() * quat(rot)));


	// FAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 200.f;
	farShadowMap.update(lightPos, vec3(0.f));
	farShadowMap.render(gameState, "s", lightPos, callback_ptr);

	// NEAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 40.f;
	nearShadowMap.update(lightPos, player_pos);
	nearShadowMap.render(gameState, "s", lightPos, callback_ptr);

	// TOON OUTLINE (Landscape)
	if (outlineMap.getWidth() != callback_ptr->xres || outlineMap.getHeight() != callback_ptr->yres) {
		outlineMap = FBuffer(callback_ptr->xres, callback_ptr->yres, "o");
	}
	outlineMap.update(projection, view);
	outlineMap.render(gameState, "t", lightPos, callback_ptr);

	// TOON OUTLINE (Objects)
	if (outlineMapNoLandscape.getWidth() != callback_ptr->xres || outlineMapNoLandscape.getHeight() != callback_ptr->yres) {
		outlineMapNoLandscape = FBuffer(callback_ptr->xres, callback_ptr->yres, "o");
	}
	outlineMapNoLandscape.update(projection, view);
	outlineMapNoLandscape.render(gameState, "l", lightPos, callback_ptr);

	// OUTLINE TO TEXTURE
	if (outlineToTexture.getWidth() != callback_ptr->xres || outlineToTexture.getHeight() != callback_ptr->yres) {
		outlineToTexture = FBuffer(callback_ptr->xres, callback_ptr->yres, "ot");
	}

	outlineToTexture.shader.use();
	outlineToTexture.shader.setInt("outline1", 1);
	outlineToTexture.shader.setInt("outline2", 2);
	outlineToTexture.shader.setMat4("projMatrixInv", glm::inverse(projection));
	outlineToTexture.shader.setMat4("viewMatrixInv", glm::inverse(view));
	outlineToTexture.shader.setFloat("outlineSensitivity", outlineSensitivity);
	outlineToTexture.shader.setFloat("fogDepth", fogDepth);
	outlineToTexture.shader.setVec3("fogColor", fogColor);
	glBindFramebuffer(GL_FRAMEBUFFER, outlineToTexture.FBO[0]);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bindTexture(1, outlineMap.fbTextures[0]);
	bindTexture(2, outlineMapNoLandscape.fbTextures[0]);
	outlineToTexture.renderQuad(outlineToTexture.fbTextures[0], 0.1f);
	glEnable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//outlineToTexture.renderToScreen(outlineToTexture.fbTextures[0]);

	// SCENE TO TEXTURE
	celMap.update(projection, view);
	bindTexture(1, nearShadowMap.fbTextures[0]);
	bindTexture(2, farShadowMap.fbTextures[0]);
	bindTexture(3, outlineMap.fbTextures[0]);
	bindTexture(4, outlineMapNoLandscape.fbTextures[0]);
	bindTexture(5, nearShadowMap.fbTextures[1]);
	bindTexture(6, farShadowMap.fbTextures[1]);
	setCelShaderUniforms(&celMap.shader);
	celMap.render(gameState, "c", lightPos, callback_ptr);
	glBindFramebuffer(GL_FRAMEBUFFER, celMap.FBO[0]);
	glDepthMask(GL_FALSE);
	glViewport(0, 0, 1920, 1080);
	testParticles.Update(timer->getDeltaTime(), fps, gameState->findEntity("platform_center")->transform->getPosition(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 15.8f));
	testParticles.Draw(view, projection, camera_previous_position);
	glViewport(0, 0, callback_ptr->xres, callback_ptr->yres);
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, celMap.FBO[0]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateBuffer.FBO[0]);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		bindTexture(1, first_iteration ? intermediateBuffer.fbTextures[1] : blurMap.fbTextures[!horizontal]);
		blurMap.renderQuad(blurMap.fbTextures[0], 0.1f);
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	bindTexture(1, intermediateBuffer.fbTextures[0]);
	bindTexture(2, blurMap.fbTextures[0]);
	bindTexture(3, outlineToTexture.fbTextures[0]);

	celMap.debugShader.use();
	celMap.debugShader.setFloat("outlineTransparency", outlineTransparency);
	celMap.debugShader.setFloat("outlineBlur", outlineBlur);
	celMap.renderToScreen(intermediateBuffer.fbTextures[0], 0.99f);
	//blurMap.renderToScreen();
	//outlineMap.renderToScreen();

	// SIXTH PASS: GUI RENDER
	textShader.use();
	mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptr->xres), 0.0f, static_cast<float>(callback_ptr->yres));
	textShader.setMat4("projection", textProjection);

	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.5);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 8.f, callback_ptr->yres - 32.f, 0.6f, vec3(0.2, 0.2f, 0.2f), textChars);

	// Game Ended Screen
	if (gameState->gameEnded) {
		string winnerText = "Tie Game!";
		if (!gameState->winner == NULL) {
			string winnerNum(1, gameState->winner->name.back());
			winnerNum = to_string(stoi(winnerNum) + 1);
			winnerText = "Salvager #" + winnerNum + " Wins!";
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
		// Ayyylien
		drawUI(testTexture, callback_ptr->xres - 300.f, 30.f, callback_ptr->xres - 30.f, 300.f, 0);

		// Boost Meter
		/*for (int i = 0; i < 10; i++) {
			int boost_meter = (int)gameState->findEntity("vehicle_0")->playerProperties->boost_meter;
			int offset = 50 * i;
			int boostoffset = 10 * i;

			if (boost_meter > boostoffset && i < 7) {
				drawUI(boostBlue, 50.0f + offset, 50.f, 100.0f + offset, 100.f, 0);
			}
			else if (boost_meter > boostoffset) {
				drawUI(boostOrange, 50.0f + offset, 50.f, 100.0f + offset, 100.f, 0);
			}
			else {
				drawUI(boostGrey, 50.0f + offset, 50.f, 100.0f + offset, 100.f, 0);
			}
		}
		*/

		// Dot Boost Meter
		for (int i = 0; i < 49; i++) {
			int boost_meter = (int)gameState->findEntity("vehicle_0")->playerProperties->boost_meter;
			int offset = 12 * i;
			float onMeter = (float)i / 65.0f;
			float actualMeter = (float)boost_meter / 100.0f;
			//int boostoffset = 10 * i;

			if (actualMeter > onMeter) {
				drawUI(boostOn, 50.0f + offset, 50.f, 62.0f + offset, 100.f, 1);
			}
			else {
				drawUI(boostOff, 50.0f + offset, 50.f, 62.0f + offset, 100.f, 1);
			}
		}

		// Player Tracker UI
		for (int i = 0; i < 6; i++) {
			float cx;
			float cy;
			float s[4];
			std::string playerString = "ui_p";
			glm::vec3 targetPos = gameState->findEntity("vehicle_" + to_string(i))->transform->getPosition();
			glm::mat4 MVP = projection * view * model;

			s[0] = (targetPos.x * MVP[0][0]) + (targetPos.y * MVP[1][0]) + (targetPos.z * MVP[2][0]) + MVP[3][0];
			s[1] = (targetPos.x * MVP[0][1]) + (targetPos.y * MVP[1][1]) + (targetPos.z * MVP[2][1]) + MVP[3][1];
			s[2] = (targetPos.x * MVP[0][2]) + (targetPos.y * MVP[1][2]) + (targetPos.z * MVP[2][2]) + MVP[3][2];
			s[3] = (targetPos.x * MVP[0][3]) + (targetPos.y * MVP[1][3]) + (targetPos.z * MVP[2][3]) + MVP[3][3];

			cx = s[0] / s[3] * callback_ptr->xres / 2 + callback_ptr->xres / 2;
			cy = s[1] / s[3] * callback_ptr->yres / 2 + callback_ptr->yres / 2;

			drawUI(ui_player_tracker[i], cx - 20.0f, cy + 40.0f, cx + 20.0f, cy + 80.0f, i);
		}
		





		// Pod Counter
		for (int i = 0; i < 10; i++) {
			int pod_count = gameState->findEntity("vehicle_0")->nbChildEntities;
			int offset = 60 * i;
			int boostoffset = 10 * i;

			if (pod_count > i) {
				drawUI(podcounterOn, 50.0f + offset, 130.f, 100.0f + offset, 180.f, 0);
			}
			else {
				drawUI(podcounterOff, 50.0f + offset, 130.f, 100.0f + offset, 180.f, 0);
			}
		}

		//drawUI(boostBlue, callback_ptr->xres - 1800.f, 30.f, callback_ptr->xres - 1730.f, 100.f, 0);


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

		// Display boost meter - deprecated
		/*
		RenderText(textShader, textVAO, textVBO, "Boost Meter: " + to_string((int)playerEntity->playerProperties->boost_meter),
			20,
			40, 0.6f,
			vec3(0.2, 0.2f, 0.2f),
			textChars);
		*/

		// Display player scores
		for (int i = 0; i < gameState->numVehicles; i++) {
			string vehicleName = "vehicle_" + to_string(i);
			string scoreText = "Salvager #" + to_string(i+1) + ": " + to_string(gameState->findEntity(vehicleName)->playerProperties->getScore());
			RenderText(textShader, textVAO, textVBO, scoreText,
				callback_ptr->xres - (16 * (int)scoreText.size()),
				callback_ptr->yres - 100.f - (i * 50.f),
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
	ImGui::SliderFloat("Outline Blur", &outlineBlur, 0.f, 1.f);
	ImGui::SliderFloat("Min bias", &minBias, 0.0f, 0.5f);
	ImGui::SliderFloat("Max bias", &maxBias, 0.0f, 0.5f);

	ImGui::Text("Camera Parameters");
	ImGui::SliderFloat("Camera Position Forward", &camera_position_forward, -200.f, 200.f);
	ImGui::SliderFloat("Camera Position Up", &camera_position_up, -200.f, 200.f);
	ImGui::SliderFloat("Camera Position Right", &camera_position_right, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Forward", &camera_target_forward, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Up", &camera_target_up, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Right", &camera_target_right, -30.f, 30.f);
	

	ImGui::Text("Audio");
	if (ImGui::SliderFloat("Player ", &playerVolume, 0.0f, 2.0f)) {
		gameState->audio_ptr->setVolume("vehicle_0_tire", playerVolume);
	}
	if (ImGui::SliderFloat("NPC ", &npcVolume, 0.0f, 2.0f)) {
		for (int i = 1; i < 4; i++) {
			std::string vehicleName = "vehicle_";
			vehicleName += to_string(i);
			gameState->audio_ptr->setVolume(vehicleName + "_tire", npcVolume);
		}
	}

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
	(*shader).setInt("nearShadowMap2", 5);
	(*shader).setInt("farShadowMap2", 6);

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

void RenderingSystem::drawUI(unsigned int texture, float x0, float y0, float x1, float y1, int l) {
	float layer = 0.05f + ((float)l / 10.f);
	x0 /= callback_ptr->xres; x0 *= 2.f; x0 -= 1.f;
	y0 /= callback_ptr->yres; y0 *= 2.f; y0 -= 1.f;
	x1 /= callback_ptr->xres; x1 *= 2.f; x1 -= 1.f;
	y1 /= callback_ptr->yres; y1 *= 2.f; y1 -= 1.f;
	celMap.debugShader.use();
	celMap.debugShader.setBool("UI", true);
	bindTexture(1, texture);
	celMap.renderQuad(texture, layer, x0, y0, x1, y1);
}