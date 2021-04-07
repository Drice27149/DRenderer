#pragma once

#include <iostream>
#include <vector>
#include "Global.hpp"

class Loader{
public:
	void loadFile(std::string filename);
    void AssimpLoadFile(std::string filename);
public: 
    std::vector<glm::vec3> vertices; // vertex coordinates
    std::vector<glm::vec2> stCoords; // texture corrdinates
    std::vector<glm::vec3> normals; // normals
    std::vector<int> vIndex; // index for vertex coordinates
    std::vector<int> stIndex; // index for texture coordinates
    std::vector<int> nIndex; // normal index, optional
    std::vector<float> outStream; // in format of [(vertex coordinate, normal, texture coordinate)]
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
private:
    void getTangent();
};