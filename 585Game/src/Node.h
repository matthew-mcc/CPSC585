#pragma once
#include<glm.hpp>
#include<vector>
using namespace std;
class Node {

public:
	unsigned int id;
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 centroid;
	vector<pair<float, Node*>>* connections;

	glm::vec3 getCentroid(Node* node) {
		return (node->v0 + node->v1 + node->v2) / 3.f;
	}

	Node(unsigned int id, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
		this->id = id;
		this->v0 = v0;
		this->v1 = v1;
		this->v2 = v2;
		this->centroid = getCentroid(this);
		this->connections = new vector<pair<float, Node*>>();
	}
	~Node() {
		// Do I need sth in here?
	}
	
};