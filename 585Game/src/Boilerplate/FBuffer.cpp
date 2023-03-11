#define NOMINMAX
#include "FBuffer.h"
#include <stdio.h>
#include <glm.hpp>
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/Shader.h>
#include <GameState.h>

FBuffer::FBuffer() {}

// Constructor for outline map
FBuffer::FBuffer(int width, int height, std::string mode) {
	WIDTH = width;
	HEIGHT = height;

	setup(mode);

	if (mode.compare("o") == 0) {
		shader = Shader("src/Shaders/shadowVertex.txt", "src/Shaders/shadowFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.setBool("isDepth", true);
	}
	else if (mode.compare("c") == 0) {
		shader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.use();
		debugShader.setBool("isDepth", false);
		debugShader.setInt("depthMap", 1);
		debugShader.setInt("bloomMap", 2);
	}
	else if (mode.compare("b") == 0) {
		shader = Shader("src/Shaders/blurVertex.txt", "src/Shaders/blurFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.setBool("isDepth", false);
	}

	quadVAO = 0;
}

// Constructor for shadow maps
FBuffer::FBuffer(unsigned int width, unsigned int height, float x, float y, float near_plane, float far_plane) {
	WIDTH = width;
	HEIGHT = height;
	mapX = x;
	mapY = y;
	nearPlane = near_plane;
	farPlane = far_plane;

	setup("o");

	shader = Shader("src/Shaders/shadowVertex.txt", "src/Shaders/shadowFragment.txt");
	debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
	debugShader.setBool("isDepth", true);

	quadVAO = 0;
}

void FBuffer::setup(std::string mode) {
	glGenFramebuffers(2, FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);

	// Generate texture
	glGenTextures(2, fbTextures);
	glBindTexture(GL_TEXTURE_2D, fbTextures[0]);

	// Outline framebuffer
	if (mode.compare("o") == 0) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// Attach texture to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbTextures[0], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// Cel framebuffer
	else if (mode.compare("c") == 0) {
		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D, fbTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Attach texture to FBO
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fbTextures[i], 0);
		}
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	// Blur framebuffer
	else if (mode.compare("b") == 0) {
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
			glBindTexture(GL_TEXTURE_2D, fbTextures[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTextures[i], 0
			);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Update shadow map
void FBuffer::update(glm::vec3 lightPos, glm::vec3 playerPos) {
	ConfigureShaderAndMatrices(lightPos, playerPos);
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
}

// Update outline map
void FBuffer::update(glm::mat4 proj, glm::mat4 view) {
	lightSpaceMatrix = proj * view;
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
}

void FBuffer::cleanUp(std::shared_ptr<CallbackInterface> callback_ptr) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, callback_ptr->xres, callback_ptr->yres);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Draw framebuffer to screen for debug purposes
void FBuffer::renderToScreen() {
	debugShader.use();
	glActiveTexture(GL_TEXTURE0);
	renderQuad();
}

void FBuffer::ConfigureShaderAndMatrices(glm::vec3 lightPos, glm::vec3 playerPos) {
	glm::mat4 lightProjection = glm::ortho(-mapX, mapX, -mapY, mapY, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(lightPos + playerPos,
		playerPos,
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
}

void FBuffer::renderQuad() {
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.1f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.1f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.1f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.1f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, fbTextures[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void FBuffer::render(GameState* gameState, std::string mode) {
	glCullFace(GL_FRONT);
	for (int i = 0; i < gameState->entityList.size(); i++) {
		// Retrieve global position and rotation
		if (mode.compare("l") == 0 && gameState->entityList.at(i).name.compare("landscape") == 0) continue;
		vec3 position = gameState->entityList.at(i).transform->getPosition();
		quat rotation = gameState->entityList.at(i).transform->getRotation();

		// Retrieve local positions and rotations of submeshes
		for (int j = 0; j < gameState->entityList.at(i).localTransforms.size(); j++) {
			vec3 localPosition = gameState->entityList.at(i).localTransforms.at(j)->getPosition();
			quat localRotation = gameState->entityList.at(i).localTransforms.at(j)->getRotation();

			// Set model matrix
			glm::mat4 model = mat4(1.0f);
			model = translate(model, position);
			model = model * toMat4(rotation);
			model = translate(model, localPosition);
			model = model * toMat4(localRotation);
			model = scale(model, vec3(1.0f));
			shader.setMat4("model", model);

			// Draw model's meshes
			gameState->entityList.at(i).model->meshes.at(j).Draw(shader);
		}
	}
	glCullFace(GL_BACK);
}

float FBuffer::getWidth() {
	return WIDTH;
}

float FBuffer::getHeight() {
	return HEIGHT;
}

