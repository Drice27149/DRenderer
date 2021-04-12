#pragma once

#include "Global.hpp"
#include "Texture.hpp"

class Mesh{
public:
    Mesh();
    Mesh(vector<Vertex> vs, vector<unsigned int> ids, vector<string> texs, int mask); // TODO: 列表复制使用右值引用
public:
    vector<Vertex> vs;          // 顶点数组
    vector<unsigned int> ids;   // 顶点索引数组
    vector<string> texns;       // 纹理贴图对应的文件名
    int mask;                   // 纹理贴图掩码
    vector<Texture> texs;       // 加载后的纹理贴图
    GraphicData gd;             // openGL 相关数据
};

class Grid: public Mesh{
public:
    Grid(vec2  a, vec2 b, vec2 c, vec2 d, int lines);
};