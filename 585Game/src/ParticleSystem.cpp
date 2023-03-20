#include "ParticleSystem.h"
#include <gtc/matrix_transform.hpp>

ParticleSystem::ParticleSystem(){}

ParticleSystem::ParticleSystem(Shader shader, unsigned int texture, unsigned int amount)
	: shader(shader), texture(texture), amount(amount) {
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

void ParticleSystem::Update(float dt, unsigned int newParticles, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset) {
	// add particles
	for (unsigned int i = 0; i < newParticles; ++i) {
		int unusedParticle = firstUnusedParticle();
		respawnParticle(particles[unusedParticle], spawnPoint, spawnVelocity, offset);
	}
	// update all particles
	for (unsigned int i = 0; i < amount; ++i)
	{
		Particle& p = particles[i];
		p.life -= dt; // reduce life
		if (p.life > 0.0f)
		{	// particle is alive, thus update
			p.position -= p.velocity * dt;
			if (p.life <= 1.f) p.color.a = p.life;
			else if (p.life >= 3.f) p.color.a = 4.f - p.life;
			else p.color.a = 1.f;
			p.velocity += ((spawnPoint + offset + vec3(0.f, -50.f, 0.f)) - p.position) * 0.001f;
		}
	}
}

void ParticleSystem::Draw(glm::mat4 view, glm::mat4 proj) {
	// use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	shader.use();
	for (Particle particle : particles)
	{
		if (particle.life > 0.0f)
		{
			glm::mat4 model = mat4(1.0f);
			model = glm::translate(model, particle.position);
			shader.setMat4("model", model);
			shader.setMat4("view", view);
			shader.setMat4("projection", proj);
			shader.setVec4("color", particle.color);
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
	float random1 = ((rand() % 5000 - 2500) / 100.0f);
	float random2 = 0.f;
	float random3 = ((rand() % 5000 - 2500) / 100.0f);
	particle.position = spawnPoint + glm::vec3(random1, random2, random3) + offset;
	particle.color = glm::vec4(0.5f, 0.f, 0.5f, 1.0f);
	particle.life = 4.0f;
	particle.velocity = glm::vec3(random1, random2, random3) * 0.1f;
}

