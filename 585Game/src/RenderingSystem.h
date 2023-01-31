#pragma once
#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Timer.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Model.h>
#include <Boilerplate/Shadow.h>
//#include <Boilerplate/Input.h>
class RenderingSystem {

public:
	// Constructor
	RenderingSystem();

	// Initialize Renderer
	void initRenderer();

	// Update Renderer
	void updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr);

	void shutdownImgui();

	// Models vector
	vector<Model> models;

	// Window pointer
	GLFWwindow* window;

private:
	void setCelShaderUniforms();

	Shadow shadowMap;

	// Shaders
	Shader textShader;
	Shader worldShader;
	Shader outlineShader;

	// Coordinate Transformations
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// Shader Parameters
	float minBias = 0.001f;
	float maxBias = 0.007f;
	float lightRotation = 0.f;
	float band = 0.166f;
	float gradient = 0.02f;
	float shift = 0.111f;
	glm::vec3 skyColor = glm::vec3(0.94f, 0.90f, 0.88f);
	glm::vec3 lightPos = glm::vec3(sin(lightRotation), 0.5f, cos(lightRotation)) * 4.f;
	glm::vec3 lightColor = glm::vec3(0.96f, 0.87f, 0.82f);
	glm::vec3 shadowColor = glm::vec3(0.82f, 0.59f, 0.44f);

	// Text
	std::map<char, Character> textChars;
	unsigned int textVAO;
	unsigned int textVBO;
	int fps;

	// Time
	Timer* timer;

};