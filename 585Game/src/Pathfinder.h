#pragma once
#include "NavMesh.h"
#include <glm.hpp>

#include<stack>
#include<vector>
#include<set>

typedef unsigned int ui;
using namespace std;

class Pathfinder {

public:

	stack<glm::vec3>* path;
	NavMesh* navMesh;

	Pathfinder(NavMesh* navMesh);
	
	bool search(Node* src, Node* dest);
	glm::vec3 getNextWaypoint();
	bool pathEmpty();

private:
	bool isDestination(Node* src, Node* dest);
	float calculateHCost(Node* src, Node* dest);
	void tracePath(Node* src, Node* dest, map<ui, ui> parents);
	void smoothPath(vector<glm::vec3> cPoints);

};