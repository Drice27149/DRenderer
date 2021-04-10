#include "GraphicAPI.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"

void GraphicAPI::LoadMeshToGPU(Mesh& mesh)
{
    GraphicData& gd = mesh.gd;
    glGenVertexArrays(1, &(gd.VAO));
    glBindVertexArray(gd.VAO);
    // 顶点缓冲
    glGenBuffers(1, &(gd.VBO));
    glBindBuffer(GL_ARRAY_BUFFER, gd.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * (mesh.vs.size()), &(mesh.vs[0]), GL_STATIC_DRAW);
    // 索引缓冲
    glGenBuffers(1, &(gd.EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gd.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (mesh.ids.size()), &(mesh.ids[0]), GL_STATIC_DRAW);
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

void GraphicAPI::Temp_DrawMesh(Mesh& mesh, Shader& sh, GLFWwindow* window)
{
    glBindVertexArray(mesh.gd.VAO);

    mat4 model = glm::mat4(1.0);
    mat4 projection = glm::perspective(90.0, 1.0, 0.1, 2000.0);
    sh.use();
    sh.setMat4("m", model);
    sh.setMat4("vp", projection);

    while (!glfwWindowShouldClose(window))
    {
        glDrawElements(GL_TRIANGLES, mesh.ids.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

