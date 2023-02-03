#include <glad/glad.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Window.h>

class Shadow {
public:
	Shadow();
	Shadow(unsigned int width, unsigned int height, float x, float y, float near, float far);
	void update(glm::vec3 lightPos);
	void cleanUp(std::shared_ptr<CallbackInterface> callback_ptr);
	void render();

	glm::mat4 lightSpaceMatrix;
	Shader shader = Shader();
	unsigned int depthMapFBO;
	unsigned int depthMap;
	

private:
	Shader debugShader;
	void ConfigureShaderAndMatrices(glm::vec3 lightPos);
	void renderQuad();

	float mapX;
	float mapY;
	float nearPlane;
	float farPlane;

	unsigned int WIDTH;
	unsigned int HEIGHT;
	unsigned int quadVAO;
	unsigned int quadVBO;
};