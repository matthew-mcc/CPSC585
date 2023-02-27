#include "Pathfinder.h"

#include<iostream>

using namespace std;

Pathfinder::Pathfinder(NavMesh* nm) {
	this->navMesh = nm;
	this->path = new stack<glm::vec3>();
}

glm::vec3 Pathfinder::getNextWaypoint() {
	if (path->size() > 0) {
		glm::vec3 vector = path->top();
		path->pop();

		return vector;
	}
	else {
		cout << "Path is Empty :(" << endl;
	}
	
	
}


bool Pathfinder::isDestination(Node* src, Node* dest) {
	if (src->id == dest->id) {
		return true;
	}
	else {
		return false;
	}
}

float Pathfinder::calculateHCost(Node* src, Node* dest) {
	glm::vec3 srcCenter = src->v0 + src->v1 + src->v2 / 3.f;
	glm::vec3 destCenter = dest->v0 + dest->v1 + dest->v2 / 3.f;

	// get euc dist
	float dx = glm::abs(srcCenter.x - destCenter.x);
	float dy = glm::abs(srcCenter.y - destCenter.y);
	float dz = glm::abs(srcCenter.z - destCenter.z);

	return glm::sqrt(dx * dx + dy * dy + dz * dz);
}

void Pathfinder::smoothPath(vector<glm::vec3> cPoints) {
	// Don't draw a curve if only 2 points exist (straight line)
	if (cPoints.size() < 3) {
		cout << "Only 2 points --> No Smoothing Needed" << endl;
		return;
	}

	vector <glm::vec3> iVerts;
	int degree = cPoints.size();

	// Clear prev path
	while (!this->path->empty()) {
		this->path->pop();
	}

	// Chaikin to generate curve

	// No clue if these are right
	glm::vec3 c0 = cPoints[0];
	glm::vec3 c1 = cPoints[1];
	glm::vec3 c2 = cPoints[2];

	for (ui u = 0; u < 4; u++) {
		// Get first 2 points
		iVerts.push_back(cPoints[0]);
		iVerts.push_back(c2 * cPoints[0] + c2 * cPoints[1]);

		// Interpolate up to n-1 remaining points
		for (int i = 1; i < degree - 2; i++) {
			iVerts.push_back(c0 * cPoints[i] + c1 * cPoints[i + 1]);
			iVerts.push_back(c1 * cPoints[i] + c0 * cPoints[i + 1]);
		}

		// Get last 2 open points
		iVerts.push_back(c2 * cPoints[degree - 1] + c2 * cPoints[degree - 2]);
		iVerts.push_back(cPoints[degree - 1]);
		cPoints = iVerts;
		iVerts.clear();
		degree = cPoints.size();
	}
}

void Pathfinder::tracePath(Node* src, Node* dest, map<ui, ui> parents) {
	/*cout << "The path is: ";*/
	vector<glm::vec3> bPath;

	// Get rid of any pre-existing paths
	while (!this->path->empty()) {
		this->path->pop();
	}

	ui temp = dest->id;
	while (temp != parents.find(temp)->second) {
		bPath.push_back(this->navMesh->nodes->find(temp)->second->centroid);
		path->push(this->navMesh->nodes->find(temp)->second->centroid);
		temp = parents.find(temp)->second;
	}
	bPath.push_back(this->navMesh->nodes->find(temp)->second->centroid);
	path->push(this->navMesh->nodes->find(temp)->second->centroid);
}

bool Pathfinder::search(Node* src, Node* dest) {
	
	if (src == NULL) {
		cout << "NULL SOURCE" << endl;
		return false;
	}
	else if (dest == NULL) {
		cout << "NULL DESTINATION" << endl;
		return false;
	}

	if (isDestination(src, dest)) {
		cout << "ALREADY AT DESTINATION!" << endl;
		return true;
	}

	// Search data structs

	map<ui, bool> explored;
	set<pair<float, Node*>> frontier;
	map<ui, ui> parents; 

	// add src node to frontier (0 cost)

	frontier.insert(make_pair(0.f, src));

	// And to explored
	explored.insert({ frontier.begin()->second->id, false });

	// And to parent
	parents.insert({ src->id, src->id }); // Does this not need to be dest for second?

	// Loop !

	while (!frontier.empty()) {
		
		// Get a reference to first in frontier, and remove it
		
		pair<float, Node*> p = *frontier.begin();
		frontier.erase(frontier.begin());

		// Add to explored
		explored.find(p.second->id)->second = true;

		float gNew = 0;
		float hNew = 0;
		float fNew = 0;

		// Init connections in explored
		for (ui i = 0; i < p.second->connections->size(); i++) {
			
			if (explored.find(p.second->connections->at(i).second->id) == explored.end()) {
				explored.insert({ p.second->connections->at(i).second->id, false });
			}
		}

		// Go through all connections
		for (ui i = 0; i < p.second->connections->size(); i++) {

			//get reference
			pair<float, Node*> temp = p.second->connections->at(i);

			//Check if we at destination
			if (isDestination(temp.second, dest)) {
				parents.insert_or_assign(temp.second->id, p.second->id);
				this->tracePath(temp.second, dest, parents);
				return true;
			}

			// Have not explored this node yet
			else if (!explored.find(temp.second->id)->second) {
				// Update cost!

				// Cost so far // Cost of current edge
				gNew = p.first + temp.first; // Actual cost
				hNew = calculateHCost(temp.second, dest); // Heuristic cost
				fNew = gNew + hNew; // Total Cost

				// Add to frontier
				frontier.insert(make_pair(fNew, temp.second));
				parents.insert_or_assign(temp.second->id, p.second->id);
			}
		}

		// If we loop through and never find the destination
		cout << "THE DESTINATION CELL IS NOT FOUND" << endl;
		return false;

	}
	


};