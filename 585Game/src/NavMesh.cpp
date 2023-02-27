#include"NavMesh.h"
#include<string>

#include "Boilerplate/OBJ_Loader.h"
//#include "PhysicsSystem.h"
using namespace std;


float NavMesh::cost(Node* src, Node* dest) {
	glm::vec3 srcCenter = src->v0 + src->v1 + src->v2 / 3.f;
	glm::vec3 destCenter = dest->v0 + dest->v1 + dest->v2 / 3.f;

	// get euc dist
	float dx = glm::abs(srcCenter.x - destCenter.x);
	float dy = glm::abs(srcCenter.y - destCenter.y);
	float dz = glm::abs(srcCenter.z - destCenter.z);

	return glm::sqrt(dx * dx + dy * dy + dz * dz);
}

NavMesh::NavMesh() {

	this->nodes = new map<unsigned int, Node*>();

	// ALl y will be 0

	// I am following the tutorial nodes/zones layout
	glm::vec3 v0 = glm::vec3(210.f, 0.f, -250.f);
	glm::vec3 v1 = glm::vec3(0.f, 0.f, -250.f);
	glm::vec3 v2 = glm::vec3(-210.f, 0.f, -250.f);


	glm::vec3 v3 = glm::vec3(210.f, 0.f, 30.f);
	glm::vec3 v4 = glm::vec3(0.f, 0.f, 30.f);
	glm::vec3 v5 = glm::vec3(-210.f, 0.f, 30.f);


	// Z might want to be 260?
	glm::vec3 v6 = glm::vec3(210.f, 0.f, 250.f); // straight ahead left
	glm::vec3 v7 = glm::vec3(0.f, 0.f, 250.f); // straight ahead edge
	glm::vec3 v8 = glm::vec3(-210.f, 0.f, 250.f); // straight ahead right

	

	Node* n0 = new Node(0, v0, v3, v4);
	Node* n1 = new Node(1, v0, v1, v4);

	Node* n2 = new Node(2, v1, v4, v5);
	Node* n3 = new Node(3, v1, v2, v5);

	Node* n4 = new Node(4, v3, v7, v6);
	Node* n5 = new Node(5, v3, v4, v7);

	Node* n6 = new Node(6, v4, v8, v7);
	Node* n7 = new Node(7, v4, v5, v8);

	this->nodes->insert({ n0->id, n0 });
	this->nodes->insert({ n1->id, n1 });
	this->nodes->insert({ n2->id, n2 });
	this->nodes->insert({ n3->id, n3 });
	this->nodes->insert({ n4->id, n4 });
	this->nodes->insert({ n5->id, n5 });
	this->nodes->insert({ n6->id, n6 });
	this->nodes->insert({ n7->id, n7 });

	// Now make connections

	n0->connections->emplace_back(make_pair(cost(n0, n1), n1));
	n0->connections->emplace_back(make_pair(cost(n0, n5), n5));

	n1->connections->emplace_back(make_pair(cost(n1, n0), n0));
	n1->connections->emplace_back(make_pair(cost(n1, n4), n4));

	n2->connections->emplace_back(make_pair(cost(n2, n1), n1));
	n2->connections->emplace_back(make_pair(cost(n2, n3), n3));
	n2->connections->emplace_back(make_pair(cost(n2, n7), n7));

	n3->connections->emplace_back(make_pair(cost(n3, n2), n2));

	n4->connections->emplace_back(make_pair(cost(n4, n5), n5));

	n5->connections->emplace_back(make_pair(cost(n5, n0), n0));
	n5->connections->emplace_back(make_pair(cost(n5, n4), n4));
	n5->connections->emplace_back(make_pair(cost(n5, n6), n6));
	
	n6->connections->emplace_back(make_pair(cost(n6, n5), n5));
	n6->connections->emplace_back(make_pair(cost(n6, n7), n7));

	n7->connections->emplace_back(make_pair(cost(n7, n2), n2));
	n7->connections->emplace_back(make_pair(cost(n7, n6), n6));




}