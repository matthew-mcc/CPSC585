#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

class Shader {
	public:
		unsigned int ID;
		Shader(const char* vertexPath, const char* fragmentPath);
		void use();
		void setBool(const std::string& name, bool value);
		void setInt(const std::string& name, int value);
		void setFloat(const std::string& name, float value);
		void setMat4(const std::string& name, glm::mat4 value);
};

void initGeomVAO(float* vertices, int size, unsigned int* VAO, unsigned int* VBO);
void initTextVAO(unsigned int* VAO, unsigned int* VBO);