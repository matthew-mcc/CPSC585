#include"NavMesh.h"
#include<string>

//#include "Boilerplate/OBJ_Loader.h"
using namespace std;

NavMesh::NavMesh() {

	this->nodes = new map<unsigned int, Node*>();

	// we can make a unique id using index!
	//objl::Loader loader1;
	string objPath = "assets/models/landscape1/landscape1.obj";
	//loader.LoadFile(objPath);

	//vector<glm::vec3> vertices;
	//for (int k = 0; k < loader.LoadedMeshes[0].Vertices.size(); k++) {
	//	float x = loader.LoadedMeshes[0].Vertices[k].Position.X; // Might need to add world position... Not sure
	//	float y = loader.LoadedMeshes[0].Vertices[k].Position.Y;
	//	float z = loader.LoadedMeshes[0].Vertices[k].Position.Z;
	//	vertices.push_back(glm::vec3(x, y, z));

	//}
	

	/*for (int i = 0; i < vertices.size() - 1; i++) {

	}*/

	// Now we have all of the vertices 

}