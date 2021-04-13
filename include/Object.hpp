#pragma once

#include "Global.hpp"
#include "Mesh.hpp"

class Object{
public:
	Object();
	vector<Mesh> meshes;
	void Transform(mat4 trans);
public:
	mat4 model;
//    void LoadModel(std::string filename);
//    void LoadTexture(std::string filename);
//    void LoadNormal(std::string filename);
//    void draw(Shader* shader);
//    void Transform(glm::mat4 trans);
//    Loader loader;
//    unsigned int VAO;
//    unsigned int VBO;
//    unsigned int EBO;
//    unsigned int textureID;
//    unsigned int normalID;
//    unsigned int tVBO;
//    unsigned int btVBO;
//    std::vector<unsigned int> elements;
//    glm::mat4 model;
//private:
//    void CalculateTangent();
};