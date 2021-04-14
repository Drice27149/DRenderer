#include "Mesh.hpp"
#include "GraphicAPI.hpp"
#include "DEngine.hpp"

Mesh::Mesh()
{
    mask = 0;
}

Mesh::Mesh(vector<Vertex>vs, vector<unsigned int> ids, vector<string> texns, int mask):
vs(vs), ids(ids), texns(texns), mask(mask)
{
    // 加载顶点以及顶点数据
    GraphicAPI::LoadMesh(*this);
    // 加载纹理贴图
    texs.resize(aiTextureType_UNKNOWN + 1);
    for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
        if (mask & (1 << it)) {
            texs[it] = DEngine::GetTexMgr().GetTextureByFileName(texns[it]);
        }
    }
}

Grid::Grid(vec2  a, vec2 b, vec2 c, vec2 d, int lines)
{
    vec2 nx = normalize(b-a);
    vec2 ny = normalize(d-a);
    vec2 dx = nx * glm::length(b-a)/(float)(lines-1);
    vec2 dy = ny * glm::length(d-a)/(float)(lines-1);
    vs.resize(lines*4);
    // 行
    vec2 up = a;
    vec2 down = d;
    for(int i = 0; i < lines; i++){
        vs[2*i].vertex = (vec3(up.x,0,up.y));
        vs[2*i+1].vertex = (vec3(down.x,0,down.y));
        up += dx;
        down += dx;
    }
    // 列
    vec2 left = a;
    vec2 right = b;
    for(int i = 0; i < lines; i++){
        vs[2*i+2*lines].vertex = (vec3(left.x,0,left.y));
        vs[2*i+1+2*lines].vertex = (vec3(right.x,0,right.y));
        left += dy;
        right += dy;
    }
    ids.resize(vs.size());
    for(int i = 0; i < vs.size(); i++) ids[i] = i;

    printf("test\n");

    GraphicAPI::LoadMesh(*this);

    printf("done\n");
}


