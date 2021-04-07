#pragma once

#include <cstdio>
#include <vector>
#include <cmath>
#include <iostream>
#include "Global.hpp"
#include "Loader.hpp"
#include "Shader.hpp"

#ifndef STB_LIB
#define STB_LIB
#include "stb_image.h"
#endif

class Object{
public:
    void LoadModel(std::string filename);
    void LoadTexture(std::string filename);
    void LoadNormal(std::string filename);
    void draw(Shader* shader);
    void Transform(glm::mat4 trans);
    Loader loader;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int textureID;
    unsigned int normalID;
    unsigned int tVBO;
    unsigned int btVBO;
    std::vector<unsigned int> elements;
    glm::mat4 model;
private:
    void CalculateTangent();
};