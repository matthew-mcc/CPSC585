#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

unsigned int initVAO(float* vertices, int size);
unsigned int initVertexShader();
unsigned int initFragmentShader();
unsigned int initShaders();