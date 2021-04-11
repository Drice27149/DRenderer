#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>           
#include <assimp/postprocess.h> 
#include <vector>
#include <random>
#include <string>
#include <iostream>
#include <cstdio>
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::normalize;
using std::vector;
using std::string;
using std::cout;

// TODO: 解耦纹理和 opengl texture 的绑定
// diffuse: texture0
// depthmap: texture1
// nomalmap: texture2
// gpostion: 3
// gnormal: 4
// gcolor: 5
// ssao: 6

struct Vertex {
    vec3 v;     // 顶点坐标
    vec3 vn;    // 顶点法线
    vec2 vt;    // 顶点纹理坐标
    vec3 tg;    // tagent, 定点切线
    vec3 bg;    // bitTagent, 同上
};

struct GraphicData {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
};