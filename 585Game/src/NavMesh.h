#pragma once
#include<map>
#include<glm.hpp>
#include"Node.h"
using namespace std;

class NavMesh {

public:
	map<unsigned int, Node*>* nodes;

	NavMesh();
	~NavMesh();

	Node* findEntity(glm::vec3 pos);
};