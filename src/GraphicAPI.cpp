#include "GraphicAPI.hpp"
#include "Mesh.hpp"

void GraphicAPI::LoadMeshToGPU(Mesh& mesh, GraphicData& gd)
{
    glGenVertexArrays(1, &(gd.VAO));
    glBindVertexArray(gd.VAO);
    // 顶点缓冲
    glGenBuffers(1, &(gd.VBO));
    glBindBuffer(GL_ARRAY_BUFFER, gd.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * (mesh.vs.size()), &(mesh.vs[0]), GL_STATIC_DRAW);
    // 索引缓冲
    glGenBuffers(1, &(gd.EBO));
    glBindBuffer(GL_ARRAY_BUFFER, gd.EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * (mesh.ids.size()), &(mesh.ids[0]), GL_STATIC_DRAW);
    // 配置顶点属性, 用于shader中读取
    // 顶点坐标
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v));
    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vn));
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vt));
    // TODO: tagent, bitagent
}

void GraphicAPI::Temp_DrawMesh(Mesh& mesh)
{
    
}

