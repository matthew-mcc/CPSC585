#pragma once
#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Model.h>
#include <Boilerplate/Timer.h>
#include <Boilerplate/Shadow.h>
#include <GameState.h>
//#include <Boilerplate/Input.h>

class RenderingSystem {
public:
	// Constructor
	RenderingSystem();

	// Initialize Renderer
	void initRenderer();

	// Update Renderer
	void updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr, GameState* gameState, Timer* timer);

	void shutdownImgui();

	// Window pointer
	GLFWwindow* window;

private:
	void setCelShaderUniforms();

	Shadow nearShadowMap;
	Shadow farShadowMap;

	// Shaders
	Shader textShader;
	Shader celShader;
	Shader outlineShader;

	// Coordinate Transformations
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// World Debug
	float altitude = 0.f;

	// Shader Parameters
	float minBias = 0.001f;
	float maxBias = 0.012f;
	float lightRotation = 5.f;
	float lightAngle = 0.3f;
	float band = 0.166f;
	float gradient = 0.02f;
	float shift = 0.111f;
	glm::vec3 skyColor = glm::vec3(0.99f, 0.84f, 0.80f);
	glm::vec3 lightPos = glm::vec3(sin(lightRotation)*cos(lightAngle), sin(lightAngle), cos(lightRotation)*cos(lightAngle)) * 4.f;
	glm::vec3 lightColor = glm::vec3(1.0f, 0.88f, 0.84f);
	glm::vec3 shadowColor = glm::vec3(0.86f, 0.69f, 0.64f);

	// Text
	std::map<char, Character> textChars;
	unsigned int textVAO;
	unsigned int textVBO;
	int fps;

	// Camera Parameters
	float camera_position_default = -7.5f;
	float camera_position_forward = -7.5f;
	float camera_position_up = 2.5f;
	float camera_position_right = 0.0f;
	float camera_target_forward = 0.0f;
	float camera_target_up = 1.2f;
	float camera_target_right = 0.0f;
	glm::vec3 camera_previous_position;
	float camera_lag = 0.05f;

};