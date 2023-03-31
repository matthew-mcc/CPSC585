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
		debugShader.use();
		debugShader.setBool("isDepth", true);
	}
	else if (mode.compare("ot") == 0) {
		shader = Shader("src/Shaders/outlineVertex.txt", "src/Shaders/outlineFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.use();
		debugShader.setBool("isDepth", false);
	}
	else if (mode.compare("c") == 0) {
		shader = Shader("src/Shaders/celVertex.txt", "src/Shaders/celFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.use();
		debugShader.setBool("isDepth", false);
		debugShader.setInt("depthMap", 1);
		debugShader.setInt("bloomMap", 2);
		debugShader.setInt("outlineMap", 3);
	}
	else if (mode.compare("b") == 0) {
		shader = Shader("src/Shaders/blurVertex.txt", "src/Shaders/blurFragment.txt");
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.use();
		debugShader.setBool("isDepth", false);
	}
	else if (mode.compare("i") == 0) {
		debugShader = Shader("src/Shaders/shadowDebugVertex.txt", "src/Shaders/shadowDebugFragment.txt");
		debugShader.use();
		debugShader.setBool("isDepth", false);
		debugShader.setInt("depthMap", 1);
		debugShader.setInt("bloomMap", 2);
		debugShader.setInt("outlineMap", 3);
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

	setup("s");

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
	if (mode.compare("c") != 0) glBindTexture(GL_TEXTURE_2D, fbTextures[0]);

	// Shadow framebuffer
	if (mode.compare("s") == 0) {
		for (int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
			glBindTexture(GL_TEXTURE_2D, fbTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			// Attach texture to FBO
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbTextures[i], 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
	}

	// Outline framebuffer
	else if (mode.compare("o") == 0) {
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

	// Outline to texture framebuffer
	else if (mode.compare("ot") == 0) {
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTextures[0], 0
		);
	}

	// Cel framebuffer
	else if (mode.compare("c") == 0) {
		int samples = 4;
		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbTextures[i]);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB,
				WIDTH, HEIGHT, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Attach texture to FBO
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, fbTextures[i], 0);
		}
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
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

	else if (mode.compare("i") == 0) {
		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D, fbTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fbTextures[i], 0);
		}
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Update shadow map
void FBuffer::update(glm::vec3 lightPos, glm::vec3 playerPos) {
	ConfigureShaderAndMatrices(lightPos, playerPos);
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
}

// Update outline map
void FBuffer::update(glm::mat4 proj, glm::mat4 view) {
	lightSpaceMatrix = proj * view;
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader.setMat4("model", glm::mat4(1.0f));
}

void FBuffer::cleanUp(std::shared_ptr<CallbackInterface> callback_ptr) {
	
}

// Draw framebuffer to screen for debug purposes
void FBuffer::renderToScreen(unsigned int texture, float layer, int quad) {
	debugShader.use();
	debugShader.setBool("UI", false);
	glActiveTexture(GL_TEXTURE0);
	renderQuad(texture, layer, quad);
}

void FBuffer::ConfigureShaderAndMatrices(glm::vec3 lightPos, glm::vec3 playerPos) {
	glm::mat4 lightProjection = glm::ortho(-mapX, mapX, -mapY, mapY, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(lightPos + playerPos,
		playerPos,
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
}

void FBuffer::renderQuad(unsigned int texture, float layer, int quad) {
	if (quad == 0) renderQuad(texture, layer, -1.0, 1.0, 1.0, -1.0);
	else if (quad == 1) renderQuad(texture, layer, -1.0, 1.0, 0.0, 0.0);
	else if (quad == 2) renderQuad(texture, layer, 0.0, 1.0, 1.0, 0.0);
	else if (quad == 3) renderQuad(texture, layer, -1.0, 0.0, 0.0, -1.0);
	else if (quad == 4) renderQuad(texture, layer, 0.0, 0.0, 1.0, -1.0);
}

void FBuffer::renderQuad(unsigned int texture, float layer, float x0, float y0, float x1, float y1) {
	float quadVertices[] = {
		// positions        // texture Coords
		x0, y0, layer, 0.0f, 1.0f,	// Top left
		x0, y1, layer, 0.0f, 0.0f,	// Bottom Left
		x1, y0, layer, 1.0f, 1.0f,	// Top Right
		x1, y1, layer, 1.0f, 0.0f,	// Bottom right
	};
	if (quadVAO == 0)
	{
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
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void FBuffer::render(GameState* gameState, std::string mode, vec3 lightPos, vec2 targetRes) {
	glViewport(0, 0, WIDTH, HEIGHT);
	int buffers = 1;
	if (mode.compare("s") == 0) buffers++;
	for (int fbo = 0; fbo < buffers; fbo++) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO[fbo]);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glCullFace(GL_FRONT);
		for (int i = 0; i < gameState->entityList.size(); i++) {
			// Retrieve global position and rotation
			//if (gameState->entityList.at(i).name.compare("sky_sphere") == 0) continue;
			if (gameState->entityList.at(i).drawType == DrawType::Invisible) continue;
			if (gameState->entityList.at(i).drawType == DrawType::Decal && mode.compare("c") != 0) continue;
			if (mode.compare("l") == 0 && gameState->entityList.at(i).name.compare("landscape") == 0) continue;
			if (mode.compare("s") == 0 && fbo == 1 && (gameState->entityList.at(i).name.compare("landscape") == 0 || gameState->entityList.at(i).name.compare("landscape_background") == 0 || gameState->entityList.at(i).name.compare("oil_rigs") == 0)) continue;

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

				if (mode.compare("c") == 0) {
					// Update relative light position
					quat lightRotation = rotation;
					quat localLightRotation = localRotation;
					lightRotation.w *= -1.f;
					localLightRotation *= -1.f;
					vec3 newLight = (vec3)(toMat4(lightRotation) * vec4(lightPos, 0.f) * toMat4(localLightRotation));
					shader.setVec3("sun", newLight);
					if (gameState->entityList.at(i).name.compare("landscape") == 0) {
						//shader.setBool("renderingLand", true);
						shader.setFloat("maxBias", 0.006f);
					}
					else {
						shader.setBool("renderingLand", false);
						shader.setFloat("maxBias", 0.002f);
					}
				}

				// Draw model's meshes
				gameState->entityList.at(i).model->meshes.at(j).Draw(shader);
			}
		}
		glCullFace(GL_BACK);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, (int)targetRes.x, (int)targetRes.y);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

float FBuffer::getWidth() {
	return WIDTH;
}

float FBuffer::getHeight() {
	return HEIGHT;
}

