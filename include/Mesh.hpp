#pragma once

#include "Global.hpp"
#include "Texture.hpp"

class Mesh{
public:
    Mesh(vector<Vertex> vs, vector<unsigned int> ids, vector<string> texs, int mask); // TODO: 列表复制使用右值引用
public:
    vector<Vertex> vs;          // 顶点数组
    vector<unsigned int> ids;   // 顶点索引数组
    vector<string> texns;        // 纹理贴图对应的文件名
    int mask;                   // 纹理贴图掩码
    vector<Texture> texs;
    // openGL 相关数据
    GraphicData gd;
};