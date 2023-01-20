#pragma once

#include<glm.hpp>
#include<vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
};

class Model {
public:
	std::vector<Vertex> vertices;
	glm::mat4 modelMatrix;
};