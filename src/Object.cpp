#include "Object.hpp"

Object::Object()
{

}

//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//// #define STB_IMAGE_IMPLEMENTATION
//#include "Object.hpp"
//
//
//void Object::LoadModel(std::string filename)
//{
//    loader.loadFile(filename);
//    glGenVertexArrays(1, &VAO);
//    glBindVertexArray(VAO);
//    glGenBuffers(1, &VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO); // VBO -> GL_ARRAY_BUFFER
//    glBufferData(GL_ARRAY_BUFFER, loader.outStream.size() * sizeof(float), loader.outStream.data(), GL_STATIC_DRAW);
//    // parm order: vertex coord, st coord, vertex normal
//    // parm id, parm size cnt, parm type, normalized, step length, start offset
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0)); // v coord
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3)); // st coord
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 5)); // v normal
//    glEnableVertexAttribArray(2);
//    // tangent and bitangent for normal mapping
//    glGenBuffers(1, &tVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, tVBO);
//    glBufferData(GL_ARRAY_BUFFER, loader.tangents.size()*sizeof(glm::vec3), loader.tangents.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0)); // v coord
//    glEnableVertexAttribArray(3);
//    // bit tangent
//    glGenBuffers(1, &btVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, btVBO);
//    glBufferData(GL_ARRAY_BUFFER, loader.bitangents.size()*sizeof(glm::vec3), loader.bitangents.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0)); // v coord
//    glEnableVertexAttribArray(4);
//    // init model transform
//    model = glm::mat4(1.0);
//}
//
//void Object::LoadTexture(std::string filename)
//{   
//    // glGenTextures(1, &textureID);
//    // glBindTexture(GL_TEXTURE_2D, textureID);
//    // int width, height, nrChannels;
//    // stbi_set_flip_vertically_on_load(true);  // reverse picture upside down
//    // unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
//    // if (data) {
//    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//    //     glGenerateMipmap(GL_TEXTURE_2D); 
//    // }
//    // else{
//    //     std::cout << "Failed to load texture" << std::endl;
//    // }
//}
//
//void Object::Transform(glm::mat4 trans)
//{
//    model = trans * model;
//}
//
//void Object::draw(Shader* shader)
//{
//    glBindVertexArray(VAO);
//    glActiveTexture(GL_TEXTURE0); // GL_TEXTURE0 -> GL_TEXTURE_2D
//    glBindTexture(GL_TEXTURE_2D, textureID); // textureID -> GL_TEXTURE_2D, in fact textureID -> GL_TEXTURE0
//    shader->setInt("texture0", 0); // GL_TEXTURE0 -> texture0, that is textureID -> texture0
//    shader->setMat4("model", model);
//    // set up normal map
//    glActiveTexture(GL_TEXTURE2);
//    glBindTexture(GL_TEXTURE_2D, normalID);
//    shader->setInt("normalMap", 2);
//    // adjust vertex normal based on model transform
//    glm::mat4 normalTrans = glm::inverse(glm::transpose(model));
//    shader->setMat4("normalTrans", normalTrans);
//    glDrawArrays(GL_TRIANGLES, 0, loader.outStream.size() / 8);
//}
//
//void Object::LoadNormal(std::string filename)
//{
//    // glGenTextures(1, &normalID);
//    // glBindTexture(GL_TEXTURE_2D, normalID);
//    // int width, height, nrChannels;
//    // // stbi_set_flip_vertically_on_load(true);  // reverse picture upside down
//    // unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
//    // if (data) {
//    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//    //     glGenerateMipmap(GL_TEXTURE_2D); 
//    // }
//    // else{
//    //     std::cout << "Failed to load texture" << std::endl;
//    // }
//}