// Includes
#include <string>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Shader.h"

using namespace std;

// Vertex Struct
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// Texture Struct
struct MTexture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // Mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<MTexture>      textures;

    // Constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<MTexture> textures);

    // Draw
    void Draw(Shader& shader);

private:
    // Rendering Data
    unsigned int VAO, VBO, EBO;

    // Mesh Setup
    void setupMesh();
};



