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
    float life;
    float size;

    Particle()
        : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f), size(0.0f) { }
};

class ParticleSystem {
public:
    ParticleSystem();
    ParticleSystem(Shader shader, unsigned int texture, unsigned int amount, float startingLife, float size, vec3 color, string mode);
    void Update(float dt, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.f), glm::vec3 offset2 = glm::vec3(0.0f, 0.0f, 0.f));
    void Draw(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition);
    void respawnParticle(Particle& particle, glm::vec3 spawnPoint, glm::vec3 spawnVelocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.f));
    void updateTex(unsigned int tex);
    vec3 color;

private:
    unsigned int firstUnusedParticle();
    std::vector<Particle> particles;
    Shader shader;
    unsigned int texture;
    unsigned int amount;
    float startingLife;
    float size = 0.2f;

    string mode;
    float timer = (startingLife + 0.1f) / (float)amount;
    
    unsigned int VAO;
};