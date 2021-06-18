#pragma once

#include "Global.hpp"
#include "Texture.hpp"

class Mesh {
public:
    Mesh();
    Mesh(vector<Vertex> vs, vector<unsigned int> ids); // TODO: 列表复制使用右值引用
public:
    vector<Vertex> vs;          // vertex buffer
    vector<unsigned int> ids;   // index buffer
    std::vector<std::string> texns; 
    int mask;
};

class Grid: public Mesh {
public:
    Grid(vec2 a = vec2(-20, 20), vec2 b = vec2(20, 20), vec2 c = vec2(20, -20), vec2 d = vec2(-20, -20), int lines = 20);
};

class Panel: public Mesh {
public:
    Panel(vec3 a = vec3(-2000, 0, -2000), vec3 b = vec3(2000, 0, -2000), vec3 c = vec3(2000, 0, 2000), vec3 d = vec3(-2000, 0, 2000));
};

class SkyBox: public Mesh {
public:
    SkyBox(float up = 200, float down = -200, float left = -200, float right = 200, float front = 200, float back = -200);
};

class Frustum: public Mesh {
public:
    Frustum(float fov, float aspect, float n, float f, int xCnt, int yCnt, int zCnt);
};