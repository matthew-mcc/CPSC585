#include "ParticleSystem.h"
#include <gtc/matrix_transform.hpp>

ParticleSystem::ParticleSystem(){}

ParticleSystem::ParticleSystem(Shader shader, unsigned int texture, unsigned int amount, float startingLife, float size, vec3 color, std::string mode)
	: shader(shader), texture(texture), amount(amount), startingLife(startingLife), size(size), color(color), mode(mode) {
	// set up mesh and attribute properties
	//test
	unsigned int VBO;
	float particle_quad[] = {
		-1.0,  1.0, 0.0, 1.0, 0.0,
		-1.0, -1.0, 0.0, 1.0, 1.0,
		 1.0,  1.0, 0.0, 0.0, 0.0,
		 1.0, -1.0, 0.0, 0.0, 1.0
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	// fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
	// set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	for (unsigned int i = 0; i < amount; i++) {
		particles.push_back(Particle());
	}
}

void ParticleSystem::Update(float dt, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset, glm::vec3 offset2) {
	// add particles
	timer += dt;
	float increment = (startingLife + 0.1f) / (float)amount;
	int newParticles = 0;
	while (timer > increment) {
		timer -= increment;
		newParticles++;
	}

	for (unsigned int i = 0; i < newParticles; ++i) {
		int unusedParticle = firstUnusedParticle();
		if (spawnVelocity != glm::vec3(0.f, 0.f, 0.f)) {
			if (rand() % 2 == 0 && mode.compare("d") == 0) respawnParticle(particles[unusedParticle], spawnPoint, spawnVelocity, offset2);
			else respawnParticle(particles[unusedParticle], spawnPoint, spawnVelocity, offset);
		}
	}
	// update all particles
	for (unsigned int i = 0; i < amount; ++i)
	{
		Particle& p = particles[i];
		p.life -= dt; // reduce life
		if (p.life > 0.0f)
		{	// particle is alive, thus update
			p.position += p.velocity * dt;
			if (p.life <= 1.f) p.color.a = p.life;
			else if (p.life >= startingLife - 1.f) p.color.a = startingLife - p.life;
			else p.color.a = 1.f;
			if (mode.compare("p") == 0) p.velocity += ((spawnPoint + offset + vec3(0.f, 30.f, 0.f)) - p.position) * 0.002f;
			else if (mode.compare("d") == 0) p.velocity += glm::vec3(0.f, -0.5f, 0.f);
		}
	}
}

void ParticleSystem::Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition) {
	// use additive blending to give it a 'glow' effect
	if (mode.compare("p") == 0) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	shader.use();
	for (Particle particle : particles)
	{
		if (particle.life > 0.0f)
		{
			glm::mat4 model = mat4(1.0f);
			shader.setMat4("model", model);
			shader.setMat4("view", view);
			shader.setMat4("projection", proj);
			shader.setVec4("color", particle.color);

			shader.setVec3("particleCenter", particle.position);
			shader.setVec3("camRight", glm::vec3(view[0][0], view[1][0], view[2][0]));
			shader.setVec3("camUp", glm::vec3(view[0][1], view[1][1], view[2][1]));
			shader.setFloat("scale", particle.size);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture);
			glActiveTexture(GL_TEXTURE0);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);
		}
	}
	// don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

unsigned int lastUsedParticle = 0;
unsigned int ParticleSystem::firstUnusedParticle() {
	// search from last used particle, this will usually return almost instantly
	for (unsigned int i = lastUsedParticle; i < amount; ++i) {
		if (particles[i].life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// otherwise, do a linear search
	for (unsigned int i = 0; i < lastUsedParticle; ++i) {
		if (particles[i].life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// override first particle if all others are alive
	lastUsedParticle = 0;
	return 0;
}

void ParticleSystem::respawnParticle(Particle& particle, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset) {
	particle.life = startingLife;
	particle.size = size;
	particle.color = vec4(color, 1.f);
	if (mode.compare("p") == 0) {
		float pos1, pos2, pos3;
		do {
			pos1 = ((rand() % 6000 - 3000) / 100.0f);
			pos2 = 0.f;
			pos3 = ((rand() % 6000 - 3000) / 100.0f);
		} while (sqrt(pow(pos1, 2.f) + pow(pos3, 2.f)) > 30.f);
		particle.position = spawnPoint + glm::vec3(pos1, pos2, pos3) + offset;
		particle.velocity = glm::vec3(pos3, pos2, -pos1) * 0.2f;
	}
	else if (mode.compare("d") == 0) {
		particle.position = spawnPoint + offset;
		particle.velocity = spawnVelocity;
	}
}
