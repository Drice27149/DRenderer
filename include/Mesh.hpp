#pragma once

#include "Global.hpp"

class Mesh{
public:
    Mesh(vector<Vertex> vs, vector<unsigned int> ids); // TODO: mesh 中增加纹理 TODO1: 列表复制使用右值引用
public:
    vector<Vertex> vs;      // 顶点数组
    vector<unsigned int> ids;   // 顶点索引数组
    GraphicData gd;
};