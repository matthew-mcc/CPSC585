#pragma once
#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Model.h>
#include <Boilerplate/Timer.h>
#include <Boilerplate/Shadow.h>
#include <GameState.h>
//#include <Boilerplate/Input.h>

using namespace glm;
using namespace std;

class RenderingSystem {
public:
	// Default Constructor
	RenderingSystem();

	// Initialize Renderer
	void initRenderer();

	void SetupImgui();

	// Update Renderer
	void updateRenderer(shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer);

	// Shutdown IMGUI Instance
	void shutdownImgui();

	// Window Pointer
	GLFWwindow* window;

private:
	void setCelShaderUniforms();

	// Shadow Maps
	Shadow nearShadowMap;
	Shadow farShadowMap;
	Shadow outlineMap;

	// Shaders
	Shader textShader;
	Shader celShader;
	Shader outlineShader;

	// Coordinate Transformations
	mat4 model = mat4(1.0f);
	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);

	// Shader Parameters
	float minBias = 0.001f;
	float maxBias = 0.002f;
	float lightRotation = 5.f;
	float lightAngle = 0.3f;
	float band = 0.166f;
	float gradient = 0.02f;
	float shift = 0.111f;
	vec3 skyColor = vec3(0.99f, 0.84f, 0.80f);
	vec3 lightPos = vec3(sin(lightRotation) * cos(lightAngle), sin(lightAngle), cos(lightRotation) * cos(lightAngle)) * 4.f;
	vec3 lightColor = vec3(0.97f, 0.91f, 0.89f);
	vec3 shadowColor = vec3(0.71f, 0.55f, 0.51f);
	vec3 fogColor = vec3(1.f, 0.73f, 0.66f);
	float fogDepth = 0.00125f;
	float outlineSensitivity = 14.264f;
	float outlineTransparency = 0.5f;

	// Text
	map<char, Character> textChars;
	unsigned int textVAO;
	unsigned int textVBO;
	int fps;

	// Camera Position / Orientation
	float camera_position_forward = -7.5f;
	float camera_position_up = 2.5f;
	float camera_position_right = 0.0f;
	float camera_target_forward = 0.0f;
	float camera_target_up = 1.2f;
	float camera_target_right = 0.0f;
	vec3 world_up = vec3(0.0f, 1.0f, 0.0f);
	vec3 camera_previous_position;

	// Camera Parameters
	float camera_lag = 0.05f;
	float fov = 45.f;
};