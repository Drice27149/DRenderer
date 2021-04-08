#pragma once

#include "Global.hpp"

class Mesh{
public:
    Mesh(vector<Vertex> vs, vector<unsigned int> ids); // TODO: mesh 中增加纹理
private:
    vector<Vertex> vs;      // 顶点数组
    vector<unsigned int> ids;   // 顶点索引数组
};