#pragma once

#include "Global.hpp"
#include "Texture.hpp"

class AssimpLoader{
public:
    AssimpLoader();
    void LoadFile(string filename);
private:
    void ProcessNode(aiNode *node, const aiScene *scene);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);
    void ProcessMaterial(aiMaterial* mat);
public:
    vector<Vertex> vs;
    vector<unsigned int> ids;
    vector<Texture> txs;
    int meshCnt;
    bool have_vn;   // 有顶点法线
    bool have_vt;   // 有顶点纹理坐标
};