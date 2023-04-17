#include "RenderingSystem.h"
#include "PhysicsSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/stb_image.h>
#include <Boilerplate/Texture.h>
#include <Camera.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define _USE_MATH_DEFINES
#include <math.h>


// Rendering System Constructor
RenderingSystem::RenderingSystem(GameState* gameState) {
	this->gameState = gameState;
	initRenderer();
}

void RenderingSystem::resetRenderer() {
	//camera_previous_position = vec3(0.0f, 8.0f, -270.0f);
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

	// TEXTURE LOADING
	for (int i = 0; i < 358; i++) {
		topSecretTextures.push_back(generateTexture(("assets/textures/Top Secret Do Not Look/alein-" + to_string(i) + ".png").c_str(), false));
	}

	testTexture = generateTexture("assets/textures/alien.png", false);
	orbTexture = generateTexture("assets/textures/orb.png", false);
	rockTexture = generateTexture("assets/textures/rock.png", false);
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

	boostBox = generateTexture("assets/textures/UI/boostBox.png", false);
	boostText = generateTexture("assets/textures/UI/boostText.png", false);

	podcounterOn = generateTexture("assets/textures/UI/podcounterOn.png", false);
	podcounterOff = generateTexture("assets/textures/UI/podCounterOff.png", false);
	podcounterPickup = generateTexture("assets/textures/UI/podcounterPickup.png", false);
	podsText = generateTexture("assets/textures/UI/podsText.png", false);

	scoreBar = generateTexture("assets/textures/UI/bar.png", false);
	scoreBarDark = generateTexture("assets/textures/UI/barDark.png", false);

	menuBackground = generateTexture("assets/textures/UI/menuBackground.png", false);
	menuTitle = generateTexture("assets/textures/UI/menuTitle.png", false);
	menuLoading = generateTexture("assets/textures/UI/menuLoading.png", false);
	menuSolo = generateTexture("assets/textures/UI/menuSolo.png", false);
	menuParty = generateTexture("assets/textures/UI/menuParty.png", false);
	menuQuit = generateTexture("assets/textures/UI/menuQuit.png", false);
	menuInfo = generateTexture("assets/textures/UI/menuInfo.png", false);
	menuInfoDisplay = generateTexture("assets/textures/UI/menuInfoDisplay.png", false);
	menuPlayerSelect2 = generateTexture("assets/textures/UI/playerSelect2.png", false);
	menuPlayerSelect3 = generateTexture("assets/textures/UI/playerSelect3.png", false);
	menuPlayerSelect4 = generateTexture("assets/textures/UI/playerSelect4.png", false);
	menuPlayerSelectBack = generateTexture("assets/textures/UI/playerSelectBack.png", false);

	backToMenu = generateTexture("assets/textures/UI/backToMenu.png", false);
	startCountdown5 = generateTexture("assets/textures/UI/startCountdown5.png", false);
	startCountdown4 = generateTexture("assets/textures/UI/startCountdown4.png", false);
	startCountdown3 = generateTexture("assets/textures/UI/startCountdown3.png", false);
	startCountdown2 = generateTexture("assets/textures/UI/startCountdown2.png", false);
	startCountdown1 = generateTexture("assets/textures/UI/startCountdown1.png", false);
	resetCountdown1 = generateTexture("assets/textures/UI/resetting1.png", false);
	resetCountdown2 = generateTexture("assets/textures/UI/resetting2.png", false);
	resetCountdown3 = generateTexture("assets/textures/UI/resetting3.png", false);
	timerAndScore = generateTexture("assets/textures/UI/timerAndScore.png", false);
	ui_timer_box = generateTexture("assets/textures/UI/timerBox.png", false);
	ui_timer_warning = generateTexture("assets/textures/UI/timerWarning.png", false);
	ui_timer_final_warning = generateTexture("assets/textures/UI/timerFinalWarning.png", false);

	ingameMenu0 = generateTexture("assets/textures/UI/ingame_menu/escapeMenu0.png", false);
	ingameMenu1 = generateTexture("assets/textures/UI/ingame_menu/escapeMenu1.png", false);
	ingameMenu2 = generateTexture("assets/textures/UI/ingame_menu/escapeMenu2.png", false);
	ingameMenu3 = generateTexture("assets/textures/UI/ingame_menu/escapeMenu3.png", false);
	volume10 = generateTexture("assets/textures/UI/ingame_menu/10-0.png", false);
	volume9 = generateTexture("assets/textures/UI/ingame_menu/9-1.png", false);
	volume8 = generateTexture("assets/textures/UI/ingame_menu/8-2.png", false);
	volume7 = generateTexture("assets/textures/UI/ingame_menu/7-3.png", false);
	volume6 = generateTexture("assets/textures/UI/ingame_menu/6-4.png", false);
	volume5 = generateTexture("assets/textures/UI/ingame_menu/5-5.png", false);
	volume4 = generateTexture("assets/textures/UI/ingame_menu/4-6.png", false);
	volume3 = generateTexture("assets/textures/UI/ingame_menu/3-7.png", false);
	volume2 = generateTexture("assets/textures/UI/ingame_menu/2-8.png", false);
	volume1 = generateTexture("assets/textures/UI/ingame_menu/1-9.png", false);
	volume0 = generateTexture("assets/textures/UI/ingame_menu/0-10.png", false);


	// Only 2 exists atm, will need to be added later like the rest
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard1.png", false));
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard2.png", false));
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard3.png", false));
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard4.png", false));
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard5.png", false));
	ui_playercard.push_back(generateTexture("assets/textures/UI/playerCard6.png", false));
	for (int i = 1; i <= 12; i++) {
		if (i <= 6) { 
			ui_player_tracker.push_back(generateTexture(("assets/textures/UI/" + to_string(i) + ".png").c_str(), false)); 
			ui_player_token.push_back(generateTexture(("assets/textures/UI/playertoken" + to_string(i) + ".png").c_str(), false));
			ui_player_token_highlight.push_back(generateTexture(("assets/textures/UI/playertokenHighlight" + to_string(i) + ".png").c_str(), false));
		}
		ui_score_tracker.push_back(generateTexture(("assets/textures/UI/cargo_indicators/" + to_string(i) + ".png").c_str(), false));

	}

	// PARTICLE SYSTEM INITIALIZATIONS
	particleShader = Shader("src/Shaders/particleVertex.txt", "src/Shaders/particleFragment.txt");
	particleShader.use();
	particleShader.setInt("sprite", 1);
	portalParticles = ParticleSystem(particleShader, orbTexture, 500, 4.0f, 0.2f, portalColor, "p");
	for (int i = 0; i < 3; i++) SHHHHITSASECRET.push_back(ParticleSystem(particleShader, testTexture, 1, 300.f, 2.0f, vec3(1.f), "a"));
	
	for (int i = 0; i < gameState->numVehicles; i++) {
		indicators.push_back(ParticleSystem(particleShader, ui_player_tracker[i], 1, 300.f, 0.f, vec3(1.f), "i"));
		indicatorCounters.push_back(ParticleSystem(particleShader, ui_player_tracker[i], 1, 300.f, 0.1f, vec3(1.f), "i"));
	}
	
	// FRAME BUFFER INITIALIZATIONS
	
	// WORLD SHADER INITIALIZATION
	stbi_set_flip_vertically_on_load(true);
	celShader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");

	// FONT INITIALIZATION
	textChars = initFont("assets/fonts/Rowdies-Regular.ttf");
	initTextVAO(&textVAO, &textVBO);
	textShader = Shader("src/Shaders/textVertex.txt", "src/Shaders/textFragment.txt");
	textShader.use();
}

// Update Renderer
void RenderingSystem::updateRenderer(vector<std::shared_ptr<CallbackInterface>> cbps, GameState* gameState, Timer* timer) {
	// CALLBACK POINTER
	numPlayers = gameState->numPlayers;
	callback_ptrs = cbps;

	if (numPlayers == 1 && nearShadowMap.size() < 1) {
		farShadowMap = FBuffer(16384, 4096, 800.f, 300.f, -700.f, 1000.f);
		nearShadowMap.push_back(FBuffer(8192, 2048, 40.f, 10.f, -500.f, 100.f));
		outlineMap.push_back(FBuffer(1920, 1080, "o"));
		outlineMapNoLandscape.push_back(FBuffer(1920, 1080, "o"));
		outlineToTexture.push_back(FBuffer(1920, 1080, "ot"));
		celMap.push_back(FBuffer(1920, 1080, "c"));
		blurMap.push_back(FBuffer(1920, 1080, "b"));
		intermediateBuffer.push_back(FBuffer(1920, 1080, "i"));
		for (int i = 0; i < gameState->numVehicles && boostParticles.size() < gameState->numVehicles; i++) {
			dirtParticles.push_back(ParticleSystem(particleShader, rockTexture, 500, 1.0f, 0.2f, dirtColor, "d"));
			if (i < numPlayers) boostParticles.push_back(ParticleSystem(particleShader, rockTexture, 1000, 0.5f, 0.25f, boostColor1, boostColor2, boostColor3, "b"));
			else boostParticles.push_back(ParticleSystem(particleShader, rockTexture, 300, 0.5f, 0.2f, boostColor1, boostColor2, boostColor3, "b"));	// Optimization: Less particles for non-player vehicles
		}
	}
	else if (nearShadowMap.size() < numPlayers) {
		nearShadowMap.pop_back();
		outlineMap.pop_back();
		outlineMapNoLandscape.pop_back();
		outlineToTexture.pop_back();
		celMap.pop_back();
		blurMap.pop_back();
		intermediateBuffer.pop_back();
		portalParticles = ParticleSystem(particleShader, orbTexture, 500 / numPlayers, 4.0f, 0.2f, portalColor, "p");
		for (int i = 0; i < gameState->numVehicles; i++) {
			dirtParticles.pop_back();
			boostParticles.pop_back();
		}
		for (int i = 0; i < gameState->numVehicles && boostParticles.size() < gameState->numVehicles; i++) {
			dirtParticles.push_back(ParticleSystem(particleShader, rockTexture, 200 / numPlayers, 1.0f, 0.2f, dirtColor, "d"));
			boostParticles.push_back(ParticleSystem(particleShader, rockTexture, 5000 / (gameState->numVehicles * numPlayers), 0.5f, 0.25f, boostColor1, boostColor2, boostColor3, "b"));
		}

		for (int i = 0; i < numPlayers; i++) {
			farShadowMap = FBuffer(16384, 4096, 800.f, 300.f, -700.f, 1000.f);
			nearShadowMap.push_back(FBuffer(4096, 1024, 40.f, 10.f, -500.f, 100.f));
			outlineMap.push_back(FBuffer(960, 540, "o"));
			outlineMapNoLandscape.push_back(FBuffer(960, 540, "o"));
			outlineToTexture.push_back(FBuffer(960, 540, "ot"));
			celMap.push_back(FBuffer(960, 540, "c"));
			blurMap.push_back(FBuffer(960, 540, "b"));
			intermediateBuffer.push_back(FBuffer(960, 540, "i"));
		}
	}

	// BACKGROUND
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);	// Set Background (Sky) Color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// RETURN TO MENU
	if (gameState->gameEnded && callback_ptrs[0]->backToMenu) {
		gameState->inMenu = true;
		gameState->gameEnded = false;
	}

	// In game escape menu
	if (!gameState->inMenu && !gameState->gameEnded && callback_ptrs[0]->ingameMenu && !callback_ptrs[0]->ingameMenuChange) {
		callback_ptrs[0]->ingameMenuChange = true;
		gameState->ingameOptionIndex = 0;
		gameState->inGameMenu = true;
		
	}
	if (!gameState->inMenu && !gameState->gameEnded && !callback_ptrs[0]->ingameMenu) {
		callback_ptrs[0]->ingameMenuChange = false;
		gameState->ingameOptionIndex = 0;
		gameState->inGameMenu = false;
	}

	//if (!gameState->gameEnded && !callback_ptrs[0]->ingameMenu) {
		//gameState->inGameMenu = false;
	//}

	// MAIN MENU
	if (gameState->inMenu) {
		// Use Text Shader
		textShader.use();
		mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptrs[0]->xres), 0.0f, static_cast<float>(callback_ptrs[0]->yres));
		textShader.setMat4("projection", textProjection);

		// Draw Background
		drawUI(menuBackground, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 2);

		// Load Screen
		if (gameState->loading) {
			drawUI(menuLoading, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
			gameState->inMenu = false;
			gameState->loading = false;
		}

		// Info Screen
		else if (gameState->showInfo) {
			drawUI(menuInfoDisplay, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
		}

		// Player Select Screen
		else if (gameState->showPlayerSelect && gameState->playerSelectIndex == 1) drawUI(menuPlayerSelectBack, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
		else if (gameState->showPlayerSelect && gameState->playerSelectIndex == 2) drawUI(menuPlayerSelect2, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
		else if (gameState->showPlayerSelect && gameState->playerSelectIndex == 3) drawUI(menuPlayerSelect3, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
		else if (gameState->showPlayerSelect && gameState->playerSelectIndex == 4) drawUI(menuPlayerSelect4, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
		
		// Menu Screen
		else {
			// Highlight Solo
			if (gameState->menuOptionIndex == 0) {
				drawUI(menuSolo, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
			}
			// Highlight Party
			if (gameState->menuOptionIndex == 1) {
				drawUI(menuParty, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
			}
			// Highlight Info
			else if (gameState->menuOptionIndex == 2) {
				drawUI(menuInfo, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
			}
			// Highlight Quit
			else if (gameState->menuOptionIndex == 3) {
				drawUI(menuQuit, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 1);
			}
			drawUI(menuTitle, 0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres, 0);
		}

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
	for (int i = playerEntities.size(); i < numPlayers && playerEntities.size() < numPlayers; i++) {
		playerEntities.push_back(gameState->findEntity("vehicle_" + to_string(i)));
		playerCameras.push_back(Camera(playerEntities[i], i));
	}
	for (int i = 0; i < numPlayers; i++) {
		playerCameras[i].updateCamera((float)timer->getDeltaTime(), callback_ptrs[i], gameState->numPlayers);
	}

	// MESH ANIMATIONS
	// Center Portal
	Entity* portalEntity = gameState->findEntity("portal_center");
	vec3 rot(0.0f, 0.3f * timer->getDeltaTime(), 0.0f);
	portalEntity->localTransforms.at(0)->setRotation(normalize(portalEntity->localTransforms.at(0)->getRotation() * quat(rot)));

	vec2 targetRes;
	if (numPlayers == 1) targetRes = vec2(callback_ptrs[0]->xres, callback_ptrs[0]->yres);
	else if (numPlayers == 2) targetRes = vec2(callback_ptrs[0]->xres, callback_ptrs[0]->yres / 2.f);
	else targetRes = vec2(callback_ptrs[0]->xres / 2.f, callback_ptrs[0]->yres / 2.f);

	gameState->listener_position = playerCameras[0].camera_previous_position;

	// FAR SHADOWMAP RENDER
	lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 200.f;
	farShadowMap.update(lightPos, vec3(0.f));
	farShadowMap.render(gameState, "s", lightPos, targetRes);

	portalParticles.Update(timer->getDeltaTime(),
		gameState->findEntity("platform_center")->transform->getPosition(),
		glm::vec3(1.f),
		glm::vec3(0.f, 0.f, 31.6f));

	for (int i = 0; i < boostParticles.size(); i++) {
		boostParticles.at(i).color = boostColor1;
		boostParticles.at(i).color2 = boostColor2;
		boostParticles.at(i).color3 = boostColor3;
		Entity* vehicleEntity = gameState->findEntity("vehicle_" + to_string(i));
		boostParticles.at(i).UpdateBoost(timer->getDeltaTime(),
			vehicleEntity->transform->getPosition() + vec3(0.f, 0.f, 0.f),
			vehicleEntity->transform->getForwardVector(),
			(vehicleEntity->transform->getLinearVelocity()) * float(vehicleEntity->playerProperties->boost != 0.f),
			vec3(toMat4(vehicleEntity->transform->getRotation()) * vec4(boostOffset, 0.f)),
			vec3(toMat4(vehicleEntity->transform->getRotation()) * vec4(-boostOffset.x, boostOffset.y, boostOffset.z, 0.f)));

		dirtParticles.at(i).Update(timer->getDeltaTime(),
			vehicleEntity->transform->getPosition() + vec3(rand() % 50 / 100.f - 0.25f, rand() % 50 / 100.f - 0.25f, rand() % 50 / 100.f - 0.25f),
			(float)vehicleEntity->transform->getOnGround()* (float)(length(vehicleEntity->transform->getLinearVelocity()) > 5.0f)* ((glm::vec3(0.f, 0.2f, 0.f)* length(vehicleEntity->transform->getLinearVelocity())) + -2.f * vehicleEntity->transform->getForwardVector() + 0.5f * vehicleEntity->transform->getLinearVelocity()),
			vec3(toMat4(vehicleEntity->transform->getRotation())* vec4(dirtOffset, 0.f)),
			vec3(toMat4(vehicleEntity->transform->getRotation())* vec4(-dirtOffset.x, dirtOffset.y, dirtOffset.z, 0.f)));
	}

	for (int i = 0; i < indicators.size(); i++) {
		indicators[i].Update(timer->getDeltaTime(),
			gameState->findEntity("vehicle_" + to_string(i))->transform->getPosition(),
			vec3(1.f),
			vec3(0.f, 3.f, 0.f));
		indicatorCounters[i].Update(timer->getDeltaTime(),
			gameState->findEntity("vehicle_" + to_string(i))->transform->getPosition(),
			vec3(1.f),
			vec3(0.f, 3.f, 0.f));
	}

	vector<vec3> secretPositions;
	secretPositions.push_back(vec3{-294.f, 46.3f, 202.13f});
	secretPositions.push_back(vec3{testPos1, testPos2, testPos3});
	secretPositions.push_back(vec3{testPos1, testPos2, testPos3});
	for (int i = 0; i < 1/*SHHHHITSASECRET.size()*/; i++) {
		SHHHHITSASECRET[i].size = 5.62f;
		SHHHHITSASECRET[i].Update(timer->getDeltaTime(),
			secretPositions[i],
			vec3(1.f),
			vec3(0.f, 3.f, 0.f));
	}

	for (int pl = 0; pl < numPlayers; pl++) {
		// NEAR SHADOWMAP RENDER
		lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 40.f;
		nearShadowMap[pl].update(lightPos, playerEntities[pl]->transform->getPosition());
		nearShadowMap[pl].render(gameState, "s", lightPos, targetRes);

		if (intermediateBuffer[pl].getWidth() != (int)targetRes.x || intermediateBuffer[pl].getHeight() != (int)targetRes.y) {
			intermediateBuffer[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "i");
		}
		// TOON OUTLINE (Landscape)
		if (outlineMap[pl].getWidth() != (int)targetRes.x || outlineMap[pl].getHeight() != (int)targetRes.y) {
			outlineMap[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "o");
		}
		outlineMap[pl].update(playerCameras[pl].projection, playerCameras[pl].view);
		outlineMap[pl].render(gameState, "t", lightPos, targetRes);

		// TOON OUTLINE (Objects)
		if (outlineMapNoLandscape[pl].getWidth() != (int)targetRes.x || outlineMapNoLandscape[pl].getHeight() != (int)targetRes.y) {
			outlineMapNoLandscape[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "o");
		}
		outlineMapNoLandscape[pl].update(playerCameras[pl].projection, playerCameras[pl].view);
		outlineMapNoLandscape[pl].render(gameState, "l", lightPos, targetRes);

		// OUTLINE TO TEXTURE
		if (outlineToTexture[pl].getWidth() != (int)targetRes.x || outlineToTexture[pl].getHeight() != (int)targetRes.y) {
			outlineToTexture[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "ot");
		}

		outlineToTexture[pl].shader.use();
		outlineToTexture[pl].shader.setInt("outline1", 1);
		outlineToTexture[pl].shader.setInt("outline2", 2);
		outlineToTexture[pl].shader.setMat4("projMatrixInv", glm::inverse(playerCameras[pl].projection));
		outlineToTexture[pl].shader.setMat4("viewMatrixInv", glm::inverse(playerCameras[pl].view));
		outlineToTexture[pl].shader.setFloat("outlineSensitivity", outlineSensitivity);
		outlineToTexture[pl].shader.setFloat("fogDepth", fogDepth);
		outlineToTexture[pl].shader.setVec3("fogColor", fogColor);
		glBindFramebuffer(GL_FRAMEBUFFER, outlineToTexture[pl].FBO[0]);
		glDisable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		bindTexture(1, outlineMap[pl].fbTextures[0]);
		bindTexture(2, outlineMapNoLandscape[pl].fbTextures[0]);
		outlineToTexture[pl].renderQuad(outlineToTexture[pl].fbTextures[0], 0.1f);
		glEnable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//outlineToTexture.renderToScreen(outlineToTexture.fbTextures[0]);

		// SCENE TO TEXTURE
		if (celMap[pl].getWidth() != (int)targetRes.x || celMap[pl].getHeight() != (int)targetRes.y) {
			celMap[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "c");
		}
		celMap[pl].update(playerCameras[pl].projection, playerCameras[pl].view);
		bindTexture(1, nearShadowMap[pl].fbTextures[0]);
		bindTexture(2, farShadowMap.fbTextures[0]);
		bindTexture(3, outlineMap[pl].fbTextures[0]);
		bindTexture(4, outlineMapNoLandscape[pl].fbTextures[0]);
		bindTexture(5, nearShadowMap[pl].fbTextures[1]);
		bindTexture(6, farShadowMap.fbTextures[1]);
		setCelShaderUniforms(&celMap[pl].shader, pl);
		celMap[pl].render(gameState, "c", lightPos, targetRes);

		portalParticles.color = portalColor;

		glBindFramebuffer(GL_FRAMEBUFFER, celMap[pl].FBO[0]);
		glDepthMask(GL_FALSE);
		glViewport(0, 0, (int)targetRes.x, (int)targetRes.y);

		particleShader.use();

		portalParticles.Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position);

		for (int i = 0; i < boostParticles.size(); i++) {
			dirtParticles.at(i).color = dirtColor;
			dirtParticles.at(i).Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position);
			boostParticles.at(i).Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position);
		}

		for (int i = 0; i < indicators.size(); i++) {
			if (i != pl) indicators[i].Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position, targetRes);

			int playerScore = gameState->findEntity("vehicle_" + to_string(i))->nbChildEntities;
			if (playerScore > 0) {
				if (playerScore > 12) playerScore = 12;
				indicatorCounters[i].updateTex(ui_score_tracker[playerScore - 1]);
				if (i != pl) indicatorCounters[i].Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position, targetRes);
			}
		}

		//printf("%4.2f, %4.2f, %4.2f\n", playerEntities[pl]->transform->getPosition().x, playerEntities[pl]->transform->getPosition().y, playerEntities[pl]->transform->getPosition().z);
		for (int i = 0; i < SHHHHITSASECRET.size(); i++) {
			SHHHHITSASECRET[i].secretTimer += timer->getDeltaTime();
			while (SHHHHITSASECRET[i].secretTimer > 0.07f) {
				SHHHHITSASECRET[i].secretTimer -= 0.07f;
				SHHHHITSASECRET[i].secretCounter++;
				if (SHHHHITSASECRET[i].secretCounter > 357) SHHHHITSASECRET[i].secretCounter = 0;
			}
			SHHHHITSASECRET[i].updateTex(topSecretTextures[SHHHHITSASECRET[i].secretCounter]);
			SHHHHITSASECRET[i].shader.use();
			SHHHHITSASECRET[i].shader.setFloat("fogDepth", fogDepth);
			SHHHHITSASECRET[i].shader.setVec3("fogColor", fogColor);
			SHHHHITSASECRET[i].Draw(playerCameras[pl].view, playerCameras[pl].projection, playerCameras[pl].camera_previous_position, targetRes);
		}
		glDepthMask(GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, celMap[pl].FBO[0]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateBuffer[pl].FBO[0]);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, targetRes.x, targetRes.y, 0, 0, targetRes.x, targetRes.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, targetRes.x, targetRes.y, 0, 0, targetRes.x, targetRes.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//farShadowMap.renderToScreen();		//Uncomment to see the far shadow map (light's perspective, near the car)
		//nearShadowMap.renderToScreen();		//Uncomment to see the near shadow map (light's perspective, the entire map)
		//outlineMap.renderToScreen();			//Uncomment to see the outline map (camera's position, just a depth map)
		//outlineMapNoLandscape.renderToScreen();
		
		// BLUR IMAGE FOR BLOOM
		if (blurMap[pl].getWidth() != (int)targetRes.x || blurMap[pl].getHeight() != (int)targetRes.y) {
			blurMap[pl] = FBuffer((int)targetRes.x, (int)targetRes.y, "b");
		}

		bool horizontal = true, first_iteration = true;
		int amount;
		if (numPlayers == 1) amount = 10;
		else amount = 2;
		blurMap[pl].shader.use();
		blurMap[pl].shader.setInt("image", 1);
		for (unsigned int i = 0; i < amount; i++)
		{
			blurMap[pl].shader.use();
			glBindFramebuffer(GL_FRAMEBUFFER, blurMap[pl].FBO[horizontal]);
			blurMap[pl].shader.setInt("horizontal", horizontal);
			bindTexture(1, first_iteration ? intermediateBuffer[pl].fbTextures[1] : blurMap[pl].fbTextures[!horizontal]);
			blurMap[pl].renderQuad(blurMap[pl].fbTextures[0], 0.1f, 0);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, callback_ptrs[0]->xres, callback_ptrs[0]->yres);

		bindTexture(1, intermediateBuffer[pl].fbTextures[0]);
		bindTexture(2, blurMap[pl].fbTextures[0]);
		bindTexture(3, outlineToTexture[pl].fbTextures[0]);

		celMap[pl].debugShader.use();
		celMap[pl].debugShader.setFloat("outlineTransparency", outlineTransparency);
		celMap[pl].debugShader.setFloat("outlineBlur", outlineBlur);
		if (numPlayers == 1) celMap[pl].renderToScreen(intermediateBuffer[pl].fbTextures[0], 0.99f, 0);
		else if (numPlayers == 2) celMap[pl].renderToScreen(intermediateBuffer[pl].fbTextures[0], 0.99f, pl+5);
		else celMap[pl].renderToScreen(intermediateBuffer[pl].fbTextures[0], 0.99f, pl+1);
		//blurMap.renderToScreen();
		//outlineMap.renderToScreen();
	}

	// SIXTH PASS: GUI RENDER
	textShader.use();
	mat4 textProjection = ortho(0.0f, static_cast<float>(callback_ptrs[0]->xres), 0.0f, static_cast<float>(callback_ptrs[0]->yres));
	textShader.setMat4("projection", textProjection);

	// Fetch and display FPS
	int fpsTest = timer->getFPS(0.5);				// Get fps (WARNING: can be NULL!)
	if (fpsTest != NULL) fps = fpsTest;				// Set fps if fpsTest isn't null
	RenderText(textShader, textVAO, textVBO, "FPS: " + std::to_string(fps), 10.f, callback_ptrs[0]->yres - 26.f, 0.45f, vec3(0.2, 0.2f, 0.2f), textChars);

	// Fetch player entity
	Entity* playerEntity = gameState->findEntity("vehicle_0");

	// Game Ended Screen
	if (gameState->gameEnded) {
		string winnerText = "Tie Game!";
		if (!gameState->winner == NULL) {
			string winnerNum(1, gameState->winner->name.back());
			winnerNum = to_string(stoi(winnerNum) + 1);
			winnerText = "Salvager #" + winnerNum + " Wins!";
		}
		drawUI(backToMenu, callback_ptrs[0]->xres / 2 - 387, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 387, callback_ptrs[0]->yres / 2 + 415, 1);
		RenderText(textShader, textVAO, textVBO, winnerText,
			callback_ptrs[0]->xres / 2 - (16 * winnerText.length()),
			callback_ptrs[0]->yres / 2 + 328,
			1.25f,
			vec3(0.94, 0.94f, 0.94f),
			textChars);
	}

	// 
	if (gameState->inGameMenu) {
		// Resume
		if (gameState->ingameOptionIndex == 0) {
			drawUI(ingameMenu0, callback_ptrs[0]->xres / 2 - 340, callback_ptrs[0]->yres / 2 - 418, callback_ptrs[0]->xres / 2 + 340, callback_ptrs[0]->yres / 2 + 418, 2);
		}
		// SFX
		if (gameState->ingameOptionIndex == 1) {
			drawUI(ingameMenu1, callback_ptrs[0]->xres / 2 - 340, callback_ptrs[0]->yres / 2 - 418, callback_ptrs[0]->xres / 2 + 340, callback_ptrs[0]->yres / 2 + 418, 2);
		}
		// Music
		if (gameState->ingameOptionIndex == 2) {
			drawUI(ingameMenu2, callback_ptrs[0]->xres / 2 - 340, callback_ptrs[0]->yres / 2 - 418, callback_ptrs[0]->xres / 2 + 340, callback_ptrs[0]->yres / 2 + 418, 2);
		}
		// Quit
		if (gameState->ingameOptionIndex == 3) {
			drawUI(ingameMenu3, callback_ptrs[0]->xres / 2 - 340, callback_ptrs[0]->yres / 2 - 418, callback_ptrs[0]->xres / 2 + 340, callback_ptrs[0]->yres / 2 + 418, 2);
		}


		// So these use ranges instead of exact values since I ran into a bug with the volume being exactly at 0, leading to some sound
		// In an abundance of caution, big ranges used
		// Drawing SFX Volume Slider:
		if (gameState->audio_ptr->SFXVolume < 0.1f) {
			drawUI(volume0, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 0.1f && gameState->audio_ptr->SFXVolume < 0.3f) {
			drawUI(volume1, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 0.3f && gameState->audio_ptr->SFXVolume < 0.5f) {
			drawUI(volume2, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 0.5f && gameState->audio_ptr->SFXVolume < 0.7f) {
			drawUI(volume3, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 0.7f && gameState->audio_ptr->SFXVolume < 0.9f) {
			drawUI(volume4, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 0.9f && gameState->audio_ptr->SFXVolume < 1.1f) {
			drawUI(volume5, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 1.1f && gameState->audio_ptr->SFXVolume < 1.3f) {
			drawUI(volume6, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 1.3f && gameState->audio_ptr->SFXVolume < 1.5f) {
			drawUI(volume7, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 1.5f && gameState->audio_ptr->SFXVolume < 1.7f) {
			drawUI(volume8, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 1.7f && gameState->audio_ptr->SFXVolume < 1.9f) {
			drawUI(volume9, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}
		if (gameState->audio_ptr->SFXVolume > 1.9) {
			drawUI(volume10, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 + 75, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 + 115, 1);
		}

		// Drawing Music Slider:
		if (gameState->audio_ptr->musicVolume < 0.1f) {
			drawUI(volume0, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 0.1f && gameState->audio_ptr->musicVolume < 0.3f) {
			drawUI(volume1, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 0.3f && gameState->audio_ptr->musicVolume < 0.5f) {
			drawUI(volume2, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 0.5f && gameState->audio_ptr->musicVolume < 0.7f) {
			drawUI(volume3, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 0.7f && gameState->audio_ptr->musicVolume < 0.9f) {
			drawUI(volume4, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 0.9f && gameState->audio_ptr->musicVolume < 1.1f) {
			drawUI(volume5, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 1.1f && gameState->audio_ptr->musicVolume < 1.3f) {
			drawUI(volume6, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 1.3f && gameState->audio_ptr->musicVolume < 1.5f) {
			drawUI(volume7, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 1.5f && gameState->audio_ptr->musicVolume < 1.7f) {
			drawUI(volume8, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 1.7f && gameState->audio_ptr->musicVolume < 1.9f) {
			drawUI(volume9, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}
		if (gameState->audio_ptr->musicVolume > 1.9f) {
			drawUI(volume10, callback_ptrs[0]->xres / 2 - 180, callback_ptrs[0]->yres / 2 - 98, callback_ptrs[0]->xres / 2 + 180, callback_ptrs[0]->yres / 2 - 58, 1);
		}


		/*
		if (gameState->ingameOptionIndex == 0) {
			drawUI(ingameMenuSelector, 
				callback_ptrs[0]->xres / 2 - 245, callback_ptrs[0]->yres / 2 + 225, 
				callback_ptrs[0]->xres / 2 + 245, callback_ptrs[0]->yres / 2 + 375, 2);
		}
		if (gameState->ingameOptionIndex == 1) {
			drawUI(ingameMenuSelector, 
				callback_ptrs[0]->xres / 2 - 245, callback_ptrs[0]->yres / 2 + 50,
				callback_ptrs[0]->xres / 2 + 245, callback_ptrs[0]->yres / 2 + 200, 2);

		}
		if (gameState->ingameOptionIndex == 2) {
			drawUI(ingameMenuSelector, 
				callback_ptrs[0]->xres / 2 - 245, callback_ptrs[0]->yres / 2 - 125,
				callback_ptrs[0]->xres / 2 + 245, callback_ptrs[0]->yres / 2 + 45, 2);
		}
		if (gameState->ingameOptionIndex == 3) {
			drawUI(ingameMenuSelector, 
				callback_ptrs[0]->xres / 2 - 245, callback_ptrs[0]->yres / 2 - 335,
				callback_ptrs[0]->xres / 2 + 245, callback_ptrs[0]->yres / 2 - 225, 2);
		}

		*/


		/*
		string debugmenu = to_string(gameState->ingameOptionIndex);
		if (gameState->ingameOptionIndex == 1) {
			string debugVolume = to_string(gameState->audio_ptr->SFXVolume);

			RenderText(textShader, textVAO, textVBO, debugVolume,
				callback_ptrs[0]->xres / 2 + 400,
				callback_ptrs[0]->yres / 2,
				3.f,
				vec3(0.94, 0.94f, 0.94f),
				textChars);
		}
		if (gameState->ingameOptionIndex == 2) {
			string debugVolume = to_string(gameState->audio_ptr->musicVolume);

			RenderText(textShader, textVAO, textVBO, debugVolume,
				callback_ptrs[0]->xres / 2 + 400,
				callback_ptrs[0]->yres / 2,
				3.f,
				vec3(0.94, 0.94f, 0.94f),
				textChars);
		}
		
		
		RenderText(textShader, textVAO, textVBO, debugmenu,
			callback_ptrs[0]->xres / 2 - 400,
			callback_ptrs[0]->yres / 2,
			3.f,
			vec3(0.94, 0.94f, 0.94f),
			textChars);
			*/

	}




	// Pre-Game Countdown
	else if (timer->getCountdown() > timer->getStartTime()) {
		int startCountdown = timer->getCountdown() - timer->getStartTime();
		switch (startCountdown) {
			case 5:	
				drawUI(startCountdown5, callback_ptrs[0]->xres / 2 - 176, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 176, callback_ptrs[0]->yres / 2 + 415, 1);
				break;
			case 4:
				drawUI(startCountdown4, callback_ptrs[0]->xres / 2 - 176, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 176, callback_ptrs[0]->yres / 2 + 415, 1);
				if (gameState->audio_ptr->lastBeep != 4) {
					gameState->audio_ptr->CountdownBeep(0, playerEntity->transform->getPosition());
					gameState->audio_ptr->lastBeep = 4;
				}
				break;
			case 3:
				drawUI(startCountdown3, callback_ptrs[0]->xres / 2 - 176, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 176, callback_ptrs[0]->yres / 2 + 415, 1);
				if (gameState->audio_ptr->lastBeep != 3) {
					gameState->audio_ptr->CountdownBeep(1, playerEntity->transform->getPosition());
					gameState->audio_ptr->lastBeep = 3;
				}
				break;
			case 2:
				drawUI(startCountdown2, callback_ptrs[0]->xres / 2 - 176, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 176, callback_ptrs[0]->yres / 2 + 415, 1);
				if (gameState->audio_ptr->lastBeep != 2) {
					gameState->audio_ptr->CountdownBeep(2, playerEntity->transform->getPosition());
					gameState->audio_ptr->lastBeep = 2;
				}
				break;
			case 1:
				drawUI(startCountdown1, callback_ptrs[0]->xres / 2 - 176, callback_ptrs[0]->yres / 2 + 158, callback_ptrs[0]->xres / 2 + 176, callback_ptrs[0]->yres / 2 + 415, 1);
				if (gameState->audio_ptr->lastBeep != 1) {
					gameState->audio_ptr->CountdownBeep(3, playerEntity->transform->getPosition());
					gameState->audio_ptr->lastBeep = 1;
				}
				break;
		}
	}
	else if (gameState->audio_ptr->lastBeep == 1) {
		gameState->audio_ptr->CountdownBeep(4, playerEntity->transform->getPosition());
		gameState->audio_ptr->lastBeep = 0;
	}

	// Normal Gameplay Screen
	else {
		// UI Scale
		// Scales relative to 2K resolution (2560 x 1440)
			// Screen width of 2560 equals scale of 1.0
			// Screen width of 1920 equals scale of 0.75
			// and so on...
		// Additionally, takes into account the number of screens (splitscreen mode) and scales accordingly
		if (gameState->numPlayers == 1) uiScale = 1.0f * (targetRes.x / 2560.f);
		if (gameState->numPlayers == 2) uiScale = 0.85f * (targetRes.x / 2560.f);
		if (gameState->numPlayers >= 3) uiScale = 1.2f * (targetRes.x / 2560.f);

		// Draw UI For Every Player's Screen
		for (int player = 0; player < gameState->numPlayers; player++) {
			
			// Calculate UI X/Y Offsets Relative to Entire Screen
			int xOffset = 0;
			int yOffset = 0;
			if (gameState->numPlayers == 2) {
				if (player == 0) yOffset = targetRes.y;
			}
			else if (gameState->numPlayers > 2) {
				if (player == 0) yOffset = targetRes.y;
				else if (player == 1) { xOffset = targetRes.x; yOffset = targetRes.y; }
				else if (player == 3) xOffset = targetRes.x;
			}

			// Boost Meter Dot Count
			int ui_bmeter_count = 30;	// Number of dot images to put in series

			// Define Margins (not scaled)
			int margin_left = 30;			// Safe area pixels from left side of screen
			int margin_right = 30;			// Safe area pixels from right side of screen
			int margin_top = 30;			// Safe area pixels from top of screen
			int margin_bottom = 30;			// Safe area pixels from bottom of screen

			// Define Relative Spacings (scaled)
			int pod_counter_height = 70;	// Y-height from bottom of screen
			int player_card_height = 140;	// Y-height from bottom of screen

			// Boost Text
			drawUI(boostText,
				margin_left + xOffset, margin_bottom + yOffset,
				margin_left + xOffset + (90 * uiScale), margin_bottom + yOffset + (60 * uiScale), 2);

			// Pods Text
			drawUI(podsText,
				margin_left + xOffset, margin_bottom + yOffset + (pod_counter_height) * uiScale,
				margin_left + xOffset + (90 * uiScale), margin_bottom + yOffset + (60 + pod_counter_height) * uiScale, 2);

			// Player Card
			int test = player;
			if (test > 1) test = 1;
			drawUI(ui_playercard[test],
				margin_left + xOffset, margin_bottom + yOffset + (player_card_height) * uiScale,
				margin_left + xOffset + (60 * uiScale), margin_bottom + yOffset + (80 + player_card_height) * uiScale, 2);

			// Boost Background Box
			drawUI(boostBox,
				margin_left + xOffset + (100 * uiScale), margin_bottom + yOffset,
				margin_left + xOffset + (540 * uiScale), margin_bottom + yOffset + (60 * uiScale), 2);
			

			// Boost Dots
			for (int i = 0; i < ui_bmeter_count; i++) {
				int boost_meter = (int)gameState->findEntity("vehicle_" + to_string(player))->playerProperties->boost_meter;
				float onMeter = (float)i / (float)ui_bmeter_count;
				float actualMeter = (float)boost_meter / 100.0f;

				if (actualMeter > onMeter) {
					drawUI(boostOn, 
						margin_left + xOffset + (110 + (i * 14)) * uiScale, margin_bottom + yOffset + (10 * uiScale),
						margin_left + xOffset + (124 + (i * 14)) * uiScale, margin_bottom + yOffset + (50 * uiScale), 1);
				}
				else {
					drawUI(boostOff, 
						margin_left + xOffset + (110 + (i * 14)) * uiScale, margin_bottom + yOffset + (10 * uiScale),
						margin_left + xOffset + (124 + (i * 14)) * uiScale, margin_bottom + yOffset + (50 * uiScale), 1);
				}
			}

			// Retrieve Stolen Pod Indices
			int stolenIndex = -1;
			int stolenIndexIndex = 0;
			if (gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices.size() > 0) {
				stolenIndex = gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices[stolenIndexIndex];
				stolenIndexIndex++;
			}

			// Drawing Pod Counter
			for (int i = 0; i < 18; i++) {
				int pod_count = gameState->findEntity("vehicle_" + to_string(player))->nbChildEntities;

				// Drawing on cargo pods
				if (i < pod_count) {
					// Drawing on top row
					if (i % 2 == 0) {
						// Deciding on stolen or not
						if (i == stolenIndex) {
							drawUI(podcounterOn, 
								margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale,
								margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (60 + pod_counter_height) * uiScale, 2);
							// Checking if there is a next stolen pod
							if (gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices.size() > stolenIndexIndex) {
								stolenIndex = gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices[stolenIndexIndex];
								stolenIndexIndex++;
							}
						}
						else
							drawUI(podcounterPickup, 
								margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale,
								margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (60 + pod_counter_height) * uiScale, 2);
					}
					// Drawing on bottom row
					else {
						if (i == stolenIndex) {
							drawUI(podcounterOn, 
								margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (pod_counter_height) * uiScale,
								margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale, 2);
							// Checking if there is a next stolen pod
							if (gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices.size() > stolenIndexIndex) {
								stolenIndex = gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices[stolenIndexIndex];
								stolenIndexIndex++;
							}
						}
						else
							drawUI(podcounterPickup, 
								margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (pod_counter_height) * uiScale,
								margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale, 2);
					}
				}
				// Drawing off cargo pods
				else {
					if (i % 2 == 0)
						drawUI(podcounterOff, 
							margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale,
							margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (60 + pod_counter_height) * uiScale, 2);
					else
						drawUI(podcounterOff, 
							margin_left + xOffset + (100 + (i * 20)) * uiScale, margin_bottom + yOffset + (pod_counter_height) * uiScale,
							margin_left + xOffset + (130 + (i * 20)) * uiScale, margin_bottom + yOffset + (30 + pod_counter_height) * uiScale, 2);
				}
			}

			// Extra Score Text
			string morescore;
			int bonus_score = gameState->calculatePoints(0, gameState->findEntity("vehicle_" + to_string(player))->nbChildEntities, gameState->findEntity("vehicle_" + to_string(player))->playerProperties->stolenTrailerIndices.size());
			morescore = "+" + to_string(bonus_score);
			RenderText(textShader, textVAO, textVBO, morescore,
				margin_left + xOffset + (480 * uiScale),
				margin_bottom + yOffset + (pod_counter_height + 15) * uiScale,
				0.8f * uiScale,
				vec3(0.93, 0.93f, 0.93f),
				textChars);

			// Create Formatted Time String
			string timerMins = std::to_string(abs(timer->getCountdownMins()));
			string timerSeconds = std::to_string(abs(timer->getCountdownSecs()));
			string overtime = "OT: ";
			string zero = "0";
			vec3 timerColour = vec3(0.93f);
			float timer_xoffset = targetRes.x / 2.f - (10.f * uiScale);

			// Calculate Time String X-Offset
			if (timerSeconds.size() < 2) {
				timerSeconds.insert(0, zero);
			}
			// Due to not fitting in the timer box, took out the extra text and offset
			//if (timer->getCountdown() < 0) {
			//	timerMins.insert(0, overtime);
			//	timer_xoffset = targetRes.x / 2.f - (100.f * uiScale);
			//}

			// Timer Box
			drawUI(ui_timer_box, 
				targetRes.x / 2 + xOffset - (130 * uiScale), targetRes.y + yOffset - (75 * uiScale), 
				targetRes.x / 2 + (130 * uiScale) + xOffset, targetRes.y + yOffset, 1);

			// Red Timer Text if Time is Running Out
			if (timer->getCountdownMins() <= 0) timerColour = vec3(0.93f, 0.0f, 0.0f);

			// Timer Text
			RenderText(textShader, textVAO, textVBO, timerMins + ":" + timerSeconds,
				xOffset + timer_xoffset + (20 * uiScale),
				targetRes.y + yOffset - (46.f * uiScale),
				0.6f * uiScale,
				timerColour,
				textChars);


			// 1 Minute Remaining Warning
			if (timer->getCountdownMins() == 0 && timer->getCountdownSecs() <= 60 && timer->getCountdownSecs() > 50) {
				drawUI(ui_timer_warning,
					targetRes.x / 2 + xOffset - (180 * uiScale), targetRes.y + yOffset - (160 * uiScale),
					targetRes.x / 2 + (180 * uiScale) + xOffset, targetRes.y + yOffset - (85 * uiScale), 1);
			}
			// 10 Second Warning
			if (timer->getCountdownMins() == 0 && timer->getCountdownSecs() <= 10 && timer->getCountdownSecs() > 0) {
				drawUI(ui_timer_final_warning,
					targetRes.x / 2 + xOffset - (180 * uiScale), targetRes.y + yOffset - (160 * uiScale),
					targetRes.x / 2 + (180 * uiScale) + xOffset, targetRes.y + yOffset - (85 * uiScale), 1);
			}



			// Scoreboard
			vector<pair<int, int>> scoreboard;
			for (int i = 0; i < gameState->numVehicles; i++) {
				scoreboard.push_back(std::make_pair(gameState->findEntity("vehicle_" + to_string(i))->playerProperties->getScore(), i));
			}
			sort(scoreboard.rbegin(), scoreboard.rend());

			// Header Text and Bar
			drawUI(scoreBar,
				targetRes.x + xOffset - margin_right - (200 * uiScale), targetRes.y + yOffset - margin_top,
				targetRes.x + xOffset - margin_right, targetRes.y + yOffset - margin_top - (40 * uiScale), 2);
			RenderText(textShader, textVAO, textVBO, "SCORE",
				targetRes.x + xOffset - margin_right - (150 * uiScale),
				targetRes.y + yOffset - margin_top - (30 * uiScale),
				0.6 * uiScale,
				vec3(0.93f),
				textChars);

			// Player Score Widget Stack
			for (int i = 0; i < gameState->numVehicles; i++) {
				// Background Bar
				if (i % 2 == 0) {
					drawUI(scoreBarDark,
						targetRes.x + xOffset - margin_right - (200 * uiScale), targetRes.y + yOffset - margin_top - (80 + (i * 40)) * uiScale,
						targetRes.x + xOffset - margin_right, targetRes.y + yOffset - margin_top - (40 + (i * 40)) * uiScale, 2);
				}
				else {
					drawUI(scoreBar,
						targetRes.x + xOffset - margin_right - (200 * uiScale), targetRes.y + yOffset - margin_top - (80 + (i * 40)) * uiScale,
						targetRes.x + xOffset - margin_right, targetRes.y + yOffset - margin_top - (40 + (i * 40)) * uiScale, 2);
				}

				// Player Number
				if (scoreboard[i].second == player) {
					drawUI(ui_player_token_highlight[scoreboard[i].second],
						targetRes.x + xOffset - margin_right - (200 * uiScale), targetRes.y + yOffset - margin_top - (80 + (i * 40)) * uiScale,
						targetRes.x + xOffset - margin_right - (165 * uiScale), targetRes.y + yOffset - margin_top - (40 + (i * 40)) * uiScale, 1);
				}
				// Non-Player Number
				else {
					drawUI(ui_player_token[scoreboard[i].second],
						targetRes.x + xOffset - margin_right - (200 * uiScale), targetRes.y + yOffset - margin_top - (80 + (i * 40)) * uiScale,
						targetRes.x + xOffset - margin_right - (165 * uiScale), targetRes.y + yOffset - margin_top - (40 + (i * 40)) * uiScale, 1);
				}

				// Score Text
				RenderText(textShader, textVAO, textVBO, ": " + to_string(scoreboard[i].first),
					targetRes.x + xOffset - margin_right - (150 * uiScale),
					targetRes.y + yOffset - margin_top - (74 + (i * 40)) * uiScale,
					0.8 * uiScale,
					vec3(0.93f),
					textChars);
			}

			// Reset Countdown
			float resetTime = playerEntities.at(player)->playerProperties->reset;
			if (resetTime != 0) {
				if (resetTime < 1.0f) {
					drawUI(resetCountdown3, 
						targetRes.x / 2 + xOffset - (176 * uiScale), targetRes.y / 2 + yOffset + (158 * uiScale), 
						targetRes.x / 2 + xOffset + (176 * uiScale), targetRes.y / 2 + yOffset + (415 * uiScale), 1);
				}
				else if (resetTime < 2.0f) {
					drawUI(resetCountdown2,
						targetRes.x / 2 + xOffset - (176 * uiScale), targetRes.y / 2 + yOffset + (158 * uiScale),
						targetRes.x / 2 + xOffset + (176 * uiScale), targetRes.y / 2 + yOffset + (415 * uiScale), 1);
				}
				else if (resetTime < 3.0f) {
					drawUI(resetCountdown1,
						targetRes.x / 2 + xOffset - (176 * uiScale), targetRes.y / 2 + yOffset + (158 * uiScale),
						targetRes.x / 2 + xOffset + (176 * uiScale), targetRes.y / 2 + yOffset + (415 * uiScale), 1);
				}
			}
		}
	}

	// Imgui Window
	ImGui::Begin("Super Space Salvagers - Debug Menu");

	ImGui::SliderFloat("Alien X", &testPos1, -350.f, -200.f);
	ImGui::SliderFloat("Alien Y", &testPos2, 40.f, 50.f);
	ImGui::SliderFloat("Alien Z", &testPos3, 150.f, 270.0f);
	ImGui::SliderFloat("Alien Scale", &testSize, 1.f, 10.0f);

	ImGui::Text("Audio");
	if (ImGui::SliderFloat("Music Volume", &gameState->audio_ptr->musicVolume, 0.0f, 3.0f)) {
		gameState->audio_ptr->setVolume("SpaceIntro", gameState->audio_ptr->musicVolume);
		gameState->audio_ptr->setVolume("SpaceMusic2", gameState->audio_ptr->musicVolume);
	}
	if (ImGui::SliderFloat("Player Engine Sounds", &gameState->audio_ptr->playerEngineVolume, 0.0f, 2.0f)) {
		gameState->audio_ptr->setVolume("vehicle_0_engine", gameState->audio_ptr->playerEngineVolume);
	}
	if (ImGui::SliderFloat("Player Tire Sounds", &gameState->audio_ptr->playerTireVolume, 0.0f, 2.0f)) {
		gameState->audio_ptr->setVolume("vehicle_0_tire", gameState->audio_ptr->playerTireVolume);
	}
	if (ImGui::SliderFloat("NPC Engine Sounds", &gameState->audio_ptr->npcEngineVolume, 0.0f, 2.0f)) {
		for (int i = 1; i < gameState->numVehicles; i++) {
			std::string vehicleName = "vehicle_";
			vehicleName += to_string(i);
			gameState->audio_ptr->setVolume(vehicleName + "_engine", gameState->audio_ptr->npcEngineVolume);
		}
	}
	if (ImGui::SliderFloat("NPC Tire Sounds", &gameState->audio_ptr->npcTireVolume, 0.0f, 2.0f)) {
		for (int i = 1; i < gameState->numVehicles; i++) {
			std::string vehicleName = "vehicle_";
			vehicleName += to_string(i);
			gameState->audio_ptr->setVolume(vehicleName + "_tire", gameState->audio_ptr->npcTireVolume);
		}
	}
	if (ImGui::SliderFloat("Honk Volume", &gameState->audio_ptr->honkVolume, 0.0f, 2.0f)) {
		for (int i = 1; i < gameState->numVehicles; i++) {
			std::string vehicleName = "vehicle_";
			vehicleName += to_string(i);
			gameState->audio_ptr->setVolume(vehicleName + "_honk", gameState->audio_ptr->honkVolume);
		}
	}

	//ImGui::Text("UI Parameters");
	//ImGui::SliderFloat("UI Scale", &uiScale, 0.f, 3.0f);

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
	ImGui::SliderFloat("Sky Bloom", &skyBloom, 1.f, 5.f);
	ImGui::SliderFloat("Min bias", &minBias, 0.0f, 0.5f);
	ImGui::SliderFloat("Max bias", &maxBias, 0.0f, 0.5f);

	ImGui::Text("Camera Parameters");
	/*ImGui::SliderFloat("Camera Position Forward", &camera_position_forward, -200.f, 200.f);
	ImGui::SliderFloat("Camera Position Up", &camera_position_up, -200.f, 200.f);
	ImGui::SliderFloat("Camera Position Right", &camera_position_right, -30.f, 30.f);*/
	ImGui::SliderFloat("Camera Target Forward", &playerCameras[0].camera_target_forward, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Up", &playerCameras[0].camera_target_up, -30.f, 30.f);
	ImGui::SliderFloat("Camera Target Right", &playerCameras[0].camera_target_right, -30.f, 30.f);

	ImGui::Text("Particle Parameters");
	ImGui::SliderFloat("Dirt offset x", (float*)&dirtOffset.x, -2.f, 2.f);
	ImGui::SliderFloat("Dirt offset y", (float*)&dirtOffset.y, -2.f, 2.f);
	ImGui::SliderFloat("Dirt offset z", (float*)&dirtOffset.z, -2.f, 2.f);
	ImGui::SliderFloat("Boost offset x", (float*)&boostOffset.x, -2.f, 2.f);
	ImGui::SliderFloat("Boost offset y", (float*)&boostOffset.y, -2.f, 2.f);
	ImGui::SliderFloat("Boost offset z", (float*)&boostOffset.z, -2.f, 2.f);
	ImGui::ColorEdit3("Portal Color", (float*)&portalColor);
	ImGui::ColorEdit3("Dirt Color", (float*)&dirtColor);
	ImGui::ColorEdit3("Boost Color 1", (float*)&boostColor1);
	ImGui::ColorEdit3("Boost Color 2", (float*)&boostColor2);
	ImGui::ColorEdit3("Boost Color 3", (float*)&boostColor3);
	





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


void RenderingSystem::setCelShaderUniforms(Shader* shader, int pl) {
	(*shader).setMat4("model", model);
	(*shader).setMat4("view", playerCameras[pl].view);
	(*shader).setMat4("projection", playerCameras[pl].projection);
	(*shader).setMat4("nearLightSpaceMatrix", nearShadowMap[pl].lightSpaceMatrix);
	(*shader).setMat4("farLightSpaceMatrix", farShadowMap.lightSpaceMatrix);
	(*shader).setMat4("outlineSpaceMatrix", outlineMap[pl].lightSpaceMatrix);
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
	if (numPlayers > 1) (*shader).setFloat("minBias", 0.006f);
	else (*shader).setFloat("minBias", minBias);
	(*shader).setFloat("maxBias", maxBias);
	(*shader).setFloat("outlineTransparency", outlineTransparency);
	(*shader).setFloat("outlineSensitivity", outlineSensitivity);
	(*shader).setFloat("skyBloom", skyBloom);
}

void RenderingSystem::bindTexture(int location, unsigned int texture) {
	glActiveTexture(GL_TEXTURE0 + location);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE0);
}

void RenderingSystem::drawUI(unsigned int texture, float x0, float y0, float x1, float y1, int l, int pl) {
	float layer = 0.05f + ((float)l / 10.f);
	x0 /= callback_ptrs[0]->xres; x0 *= 2.f; x0 -= 1.f;
	y0 /= callback_ptrs[0]->yres; y0 *= 2.f; y0 -= 1.f;
	x1 /= callback_ptrs[0]->xres; x1 *= 2.f; x1 -= 1.f;
	y1 /= callback_ptrs[0]->yres; y1 *= 2.f; y1 -= 1.f;
	celMap[pl].debugShader.use();
	celMap[pl].debugShader.setBool("UI", true);
	bindTexture(1, texture);
	celMap[pl].renderQuad(texture, layer, x0, y0, x1, y1);
}