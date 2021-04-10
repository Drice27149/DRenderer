#include "GraphicAPI.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

void GraphicAPI::LoadMesh(Mesh& mesh)
{
    GraphicData& gd = mesh.gd;
    glGenVertexArrays(1, &(gd.VAO));
    glBindVertexArray(gd.VAO);
    // ���㻺��
    glGenBuffers(1, &(gd.VBO));
    glBindBuffer(GL_ARRAY_BUFFER, gd.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * (mesh.vs.size()), &(mesh.vs[0]), GL_STATIC_DRAW);
    // ��������
    glGenBuffers(1, &(gd.EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gd.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (mesh.ids.size()), &(mesh.ids[0]), GL_STATIC_DRAW);
    // ���ö�������, ����shader�ж�ȡ
    // ��������
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v));
    // ���㷨��
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vn));
    // ��������
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vt));
    // TODO: tagent, bitagent
}

void GraphicAPI::LoadImageTexture(Texture& tex, string fn, bool vflip)
{
    glGenTextures(1, &(tex.id));
    glBindTexture(GL_TEXTURE_2D, tex.id);
    int width, height, nrChannels;
    // ��ֱ��ת
    if (vflip) stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(fn.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
}

void GraphicAPI::Temp_DrawMesh(Mesh& mesh, Shader& sh, GLFWwindow* window)
{
    glBindVertexArray(mesh.gd.VAO);

    mat4 model = glm::mat4(1.0);
    mat4 projection = glm::perspective(90.0, 1.0, 0.1, 2000.0);
    sh.use();
    // ģ�ͱ任��͸��ͶӰ�任
    sh.setMat4("m", model);
    sh.setMat4("vp", projection);
    // ��λ������ͼ
    int texNum = 0;
    if (mesh.mask & (1 << aiTextureType_DIFFUSE)) {
        glActiveTexture(GL_TEXTURE0 + texNum);
        glBindTexture(GL_TEXTURE_2D, mesh.texs[aiTextureType_DIFFUSE].id);
        sh.setInt("modelTex.diffuse", texNum);
        texNum++;
    }
    if (mesh.mask & (1 << aiTextureType_NORMALS)) {
        glActiveTexture(GL_TEXTURE0 + texNum);
        glBindTexture(GL_TEXTURE_2D, mesh.texs[aiTextureType_NORMALS].id);
        sh.setInt("modelTex.normal", texNum);
        texNum++;
    }

    // ������Ȳ���
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_DEPTH_BUFFER_BIT |GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, mesh.ids.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

