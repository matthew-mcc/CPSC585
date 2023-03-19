#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

unsigned int generateTexture(const char* imagePath, bool isJPG);
unsigned int generateChar(FT_Face face);
