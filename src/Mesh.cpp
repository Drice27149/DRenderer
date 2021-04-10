#include "Mesh.hpp"
#include "GraphicAPI.hpp"

Mesh::Mesh(vector<Vertex>vs, vector<unsigned int> ids, vector<string> texns, int mask):
vs(vs), ids(ids), texns(texns), mask(mask)
{
    // 加载顶点以及顶点数据
    GraphicAPI::LoadMesh(*this);
    // 加载纹理贴图
    texs.resize(aiTextureType_UNKNOWN + 1);
    for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
        if (mask & (1 << it)) {
            GraphicAPI::LoadImageTexture(texs[it], texns[it]);
        }
    }
}


