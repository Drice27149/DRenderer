#pragma once

#include "Global.hpp"
#include "Texture.hpp"
#include "Object.hpp"

class AssimpLoader{
public:
    void LoadFile(Object* obj, string filename);
private:
    void ProcessNode(aiNode *node, const aiScene *scene);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);
    void ProcessMaterial(aiMaterial* mat);
public:
    vector<Vertex> vs;
    vector<unsigned int> ids;
    int meshCnt;
private:
    string fpath;
    Object* obj;
};