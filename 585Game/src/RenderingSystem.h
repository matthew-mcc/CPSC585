#pragma once
#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Text.h>
#include <Boilerplate/Model.h>
#include <Boilerplate/Timer.h>
#include <Entity.h>

//#include <Boilerplate/Input.h>
class RenderingSystem {

public:
	// Constructor
	RenderingSystem();

	// Initialize Renderer
	void initRenderer();

	// Update Renderer
	void updateRenderer(std::shared_ptr<CallbackInterface> callback_ptr, std::vector<Entity> entityList, Timer* timer);

	// Window pointer
	GLFWwindow* window;

private:
	// Shaders
	Shader textShader;
	Shader worldShader;

	// Coordinate Transformations
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// Text
	std::map<char, Character> textChars;
	unsigned int textVAO;
	unsigned int textVBO;
	int fps;

};