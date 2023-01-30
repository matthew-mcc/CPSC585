#include <glad/glad.h>
#include <Boilerplate/Shader.h>

class Shadow {
public:
	Shadow();
	Shadow(unsigned int width, unsigned int height);
	void update(glm::vec3 lightPos);
	void cleanUp();
	void render();

	glm::mat4 lightSpaceMatrix;
	Shader shader = Shader();
	unsigned int depthMapFBO;
	unsigned int depthMap;
	

private:
	Shader debugShader;
	void ConfigureShaderAndMatrices(glm::vec3 lightPos);
	void renderQuad();

	unsigned int WIDTH;
	unsigned int HEIGHT;
	unsigned int quadVAO;
	unsigned int quadVBO;
};