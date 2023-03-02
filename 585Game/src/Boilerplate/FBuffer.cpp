#include "FBuffer.h"
#include <stdio.h>
#include <glm.hpp>
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <Boilerplate/Shader.h>

FBuffer::FBuffer() {}

// Constructor for shadow maps
FBuffer::FBuffer(int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Generate texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = Shader("src/Shaders/shadowVertex.txt", "src/Shaders/shadowFragment.txt");
	debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");

	quadVAO = 0;
}

// Constructor for outline map
FBuffer::FBuffer(unsigned int width, unsigned int height, float x, float y, float near_plane, float far_plane) {
	WIDTH = width;
	HEIGHT = height;
	mapX = x;
	mapY = y;
	nearPlane = near_plane;
	farPlane = far_plane;

	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Generate texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = Shader("src/Shaders/shadowVertex.txt", "src/Shaders/shadowFragment.txt");
	debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");

	quadVAO = 0;
}

// Update shadow map
void FBuffer::update(glm::vec3 lightPos, glm::vec3 playerPos) {
	ConfigureShaderAndMatrices(lightPos, playerPos);
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
}

// Update outline map
void FBuffer::update(glm::mat4 proj, glm::mat4 view) {
	lightSpaceMatrix = proj * view;
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
	glViewport(0, 0, WIDTH, HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
}

void FBuffer::cleanUp(std::shared_ptr<CallbackInterface> callback_ptr) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, callback_ptr->xres, callback_ptr->yres);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Draw framebuffer to screen for debug purposes
void FBuffer::render() {
	debugShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
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
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
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
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


