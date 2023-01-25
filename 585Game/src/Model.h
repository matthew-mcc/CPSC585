#pragma once
#pragma once

#include<glm.hpp>
#include<vector>

/*
Should be all we need for the rendering portion. The color will likely switch to a texture, but for simplicity color for now.
*/

// Vertex (Pos + color) or (Pos + tex coord)
struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
};

// Model has a vector of vertices and a matrix for the shader
class Model {
public:
	std::vector<Vertex> vertices;
	glm::mat4 modelMatrix;
};