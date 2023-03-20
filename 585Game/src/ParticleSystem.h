#pragma once
//#include <Boilerplate/Window.h>
#include <Boilerplate/Shader.h>
#include <vector>
//#include <Boilerplate/Text.h>
//#include <Boilerplate/Model.h>
//#include <Boilerplate/Timer.h>
//#include <Boilerplate/FBuffer.h>
//#include <GameState.h>
//#include <Boilerplate/Input.h>

using namespace glm;
using namespace std;

struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color;
    float     life;

    Particle()
        : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) { }
};

class ParticleSystem {
public:
    ParticleSystem();
    ParticleSystem(Shader shader, unsigned int texture, unsigned int amount);
    void Update(float dt, float framerate, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.f));
    void Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition);

private:
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.f));
    std::vector<Particle> particles;
    Shader shader;
    unsigned int texture;
    unsigned int amount;
    
    unsigned int VAO;
};