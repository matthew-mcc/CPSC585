#include <glad/glad.h>
#include <Boilerplate/Shader.h>
#include <Boilerplate/Window.h>

class FBuffer {
public:
	FBuffer();
	FBuffer(int width, int height, std::string mode);
	FBuffer(unsigned int width, unsigned int height, float x, float y, float near, float far);
	void update(glm::vec3 lightPos, glm::vec3 playerPos);
	void update(glm::mat4 proj, glm::mat4 view);
	void cleanUp(std::shared_ptr<CallbackInterface> callback_ptr);
	void render();
	void renderQuad();

	float getWidth();
	float getHeight();

	glm::mat4 lightSpaceMatrix;
	Shader shader = Shader();
	Shader debugShader;
	unsigned int FBO[2];
	unsigned int fbTextures[2];


private:
	void ConfigureShaderAndMatrices(glm::vec3 lightPos, glm::vec3 playerPos);
	void setup(std::string mode);

	float mapX;
	float mapY;
	float nearPlane;
	float farPlane;

	unsigned int WIDTH;
	unsigned int HEIGHT;
	unsigned int quadVAO;
	unsigned int quadVBO;
};