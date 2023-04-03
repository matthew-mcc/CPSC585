#pragma once
#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Model.h>
#include <Boilerplate/Timer.h>
#include <Boilerplate/FBuffer.h>
#include <Camera.h>
#include <GameState.h>
#include <ParticleSystem.h>
//#include <Boilerplate/Input.h>

using namespace glm;
using namespace std;

class RenderingSystem {
public:
	// Default Constructor
	RenderingSystem(GameState* gameState);

	// Initialize Renderer
	void initRenderer();

	void SetupImgui();

	// Update Renderer
	void updateRenderer(vector<std::shared_ptr<CallbackInterface>> cbps, GameState* gameState, Timer* timer);

	// Shutdown IMGUI Instance
	void shutdownImgui();

	// Reset any state-sensitive variables
	void resetRenderer();

	// Window Pointer
	GLFWwindow* window;

	// GameState Pointer
	GameState* gameState;

private:
	void setCelShaderUniforms(Shader* shader, int pl);
	void bindTexture(int location, unsigned int texture);

	void drawUI(unsigned int texture, float x0, float y0, float x1, float y1, int l = 0, int pl = 0);
  
	vector<std::shared_ptr<CallbackInterface>> callback_ptrs;

	// Number of active players
	int numPlayers = 1;
	vector<Entity*> playerEntities;
	vector<Camera> playerCameras;

	// Particle Generators
	ParticleSystem portalParticles;
	std::vector<ParticleSystem> dirtParticles;
	std::vector<ParticleSystem> boostParticles;
	std::vector<ParticleSystem> indicators;
	std::vector<ParticleSystem> indicatorCounters;

	// Frame buffers
	vector<FBuffer> nearShadowMap;
	FBuffer farShadowMap;
	vector<FBuffer> outlineMap;
	vector<FBuffer> outlineMapNoLandscape;
	vector<FBuffer> outlineToTexture;
	vector<FBuffer> celMap;
	vector<FBuffer> blurMap;
	vector<FBuffer> intermediateBuffer;

	// UI Textures
	unsigned int testTexture;
	unsigned int orbTexture;
	unsigned int rockTexture;
	unsigned int boostBlue;
	unsigned int boostOrange;
	unsigned int boostGrey;
	unsigned int boostBox;
	unsigned int boostText;
	unsigned int boostOn;
	unsigned int boostOff;

	unsigned int podsText;
	unsigned int podcounterOn;
	unsigned int podcounterOff;
	unsigned int podcounterPickup;

	unsigned int scoreBarDark;
	unsigned int scoreBar;

	unsigned int menuBackground;
	unsigned int menuTitle;
	unsigned int menuLoading;
	unsigned int menuSolo;
	unsigned int menuParty;
	unsigned int menuQuit;
	unsigned int menuInfo;
	unsigned int menuInfoDisplay;
	unsigned int backToMenu;
	unsigned int menuPlayerSelect2;
	unsigned int menuPlayerSelect3;
	unsigned int menuPlayerSelect4;
	unsigned int menuPlayerSelectBack;
	unsigned int startCountdown5;
	unsigned int startCountdown4;
	unsigned int startCountdown3;
	unsigned int startCountdown2;
	unsigned int startCountdown1;
	unsigned int timerAndScore;
	unsigned int ui_timer_box;

	std::vector<unsigned int> ui_player_tracker;
	std::vector<unsigned int> ui_score_tracker;
	std::vector<unsigned int> ui_playercard;
	std::vector<unsigned int> ui_player_token;

	// Shaders
	Shader textShader;
	Shader celShader;
	Shader particleShader;

	// Coordinate Transformations
	mat4 model = mat4(1.0f);

	// Shader Parameters
	float minBias = 0.001f;
	float maxBias = 0.002f;
	float lightRotation = 5.f;
	float lightAngle = 0.3f;
	float band = 0.166f;
	float gradient = 0.02f;
	float shift = 0.111f;
	vec3 skyColor = vec3(0.613f, 0.541f, 0.532f);
	vec3 lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 4.f;
	vec3 lightColor = vec3(0.97f, 0.91f, 0.89f);
	vec3 shadowColor = vec3(0.71f, 0.55f, 0.51f);
	vec3 fogColor = vec3(1.f, 0.728f, 0.681f);
	float fogDepth = 0.00125f;

	float outlineSensitivity = 22.f;
	float outlineTransparency = 0.5f;
	float outlineBlur = 0.1f;
	float skyBloom = 1.f;

	// Text
	map<char, Character> textChars;
	unsigned int textVAO;
	unsigned int textVBO;
	int fps;

	// Camera Position / Orientation

	// Camera Parameters

	// Audio Parameters
	float playerTireVolume = 0.5f;
	float playerEngineVolume = 0.5f;
	float npcTireVolume = 1.0f;
	float npcEngineVolume = 1.0f;
	float musicVolume = 1.0f;

	vec3 dirtOffset = vec3(1.2f, -0.3f, -0.9f);
	vec3 boostOffset = vec3(0.684f, 0.773f, -0.900f);
	vec3 portalColor = vec3(0.26f, 0.58f, 0.89f);
	vec3 boostColor1 = vec3(0.5f, 0.5f, 0.5f);
	vec3 boostColor2 = vec3(0.33f, 0.33f, 0.1f);
	vec3 boostColor3 = vec3(0.1f, 0.f, 0.f);
	vec3 dirtColor = vec3(0.38f, 0.20f, 0.17f);
};